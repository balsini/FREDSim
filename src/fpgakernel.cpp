#include <algorithm>

#include <simul.hpp>

#include <cpu.hpp>
#include <fpgakernel.hpp>
#include <resmanager.hpp>
#include <scheduler.hpp>
#include <clusteredsched.hpp>
#include <task.hpp>
#include <fcfsresmanager.hpp>

namespace RTSim {

  using namespace std;
  using namespace MetaSim;

  FPGAKernel::FPGAKernel(DispatcherAlgorithm da)
  {
    _CPUFactory = new uniformCPUFactory();
    disp_alg = da;

    _resMng = new FCFSResManager("FPGAResourceManager");
    _resMng->addResource("ICAP");
    _resMng->setKernel(this, nullptr);
  }


  Scheduler * FPGAKernel::addPartition(int slotNum)
  {
    Scheduler * sched = new ClusteredScheduler();

    for (unsigned int i=0; i<slotNum; ++i) {
      Slot slot;

      slot.task = nullptr;
      slot.scheduler = sched;
      slot.cpu = _CPUFactory->createCPU();
      slot.cpu->setIndex(cpu_index);
      cpu_index++;
      scheduler.insert(pair<Scheduler *, Slot>(sched, slot));
    }
    //k.push_back(new RTKernel(s.back()));

    return sched;
  }


  FPGAKernel::~FPGAKernel()
  {
    while (scheduler.size() > 0) {
      std::multimap<Scheduler *, Slot>::iterator it=scheduler.begin();
      delete (*it).first;
      scheduler.erase((*it).first);
    }

    delete _resMng;
  }


  void FPGAKernel::addTask(AbsRTTask &t, const string &params)
  {
    HardwareTask *h = dynamic_cast<HardwareTask *>(&t);
    h->setKernel(this);
  }


  void FPGAKernel::activate(AbsRTTask * t)
  {
    HardwareTask *h = dynamic_cast<HardwareTask *>(t);

    h->onInstrEnd();
    // TODO
  }


  void FPGAKernel::suspend(AbsRTTask * t)
  {
    // TODO
  }


  void FPGAKernel::dispatch()
  {
    // For each free slot, assign an hardware task

    for (std::multimap<Scheduler *, Slot>::iterator s=scheduler.begin(); s!=scheduler.end(); ++s) {
      //std::cout << (*s).first << " => " << (*s).second << '\n';
      if ((*s).second.task == nullptr) {

        AbsRTTask* t = (*s).first->getFirst();

        if (t) {
          (*s).second.task = t;
          // TODO

          //HardwareTask *hw = dynamic_cast<HardwareTask *>(t);

          //          hw->setIndex(0);

          (*s).first->notify(t);
          t->schedule();
          (*s).first->removeTask(t);
        }
        //h->setKernel(this);
        //k[h->getAffinity()]->onArrival(t);
      }
    }

  }


  unsigned int FPGAKernel::number_of_slots(Scheduler *s)
  {
    unsigned int counter = 0;

    pair <multimap<Scheduler *,Slot>::iterator,
        multimap<Scheduler *,Slot>::iterator> ret;
    ret = scheduler.equal_range(s);

    for (multimap<Scheduler *,Slot>::iterator it = ret.first; it != ret.second; ++it)
        counter++;

    return counter;
  }


  Scheduler *FPGAKernel::dispatcher(const vector<Scheduler *>&affinity)
  {
    std::vector<Scheduler *>::const_iterator it;
    std::vector<Scheduler *>::const_iterator shortest = affinity.begin();
    map<Scheduler *,unsigned int> scheduler_counter;
    map<Scheduler *,unsigned int>::iterator scheduler_counter_higher;

    switch (disp_alg) {
      case DISPATCHER_FIRST:
        return affinity.front();
        break;
      case DISPATCHER_LESS_WAITING_T:
        for (it = affinity.begin() ; it != affinity.end(); ++it) {
          if ((dynamic_cast<ClusteredScheduler *>(*it))->size()
              < (dynamic_cast<ClusteredScheduler *>(*shortest))->size())
            shortest = it;
        }
        return *shortest;
        break;
      case DISPATCHER_MORE_FREE_SLOTS:

        // Counts the number of free slots for each partition

        for (it = affinity.begin() ; it != affinity.end(); ++it) {

          if (scheduler_counter.find(*it) == scheduler_counter.end())
            scheduler_counter[*it] = 0;

          pair <multimap<Scheduler *,Slot>::iterator,
              multimap<Scheduler *,Slot>::iterator> ret;
          ret = scheduler.equal_range(*it);
          //std::cout << " =>";
          for (multimap<Scheduler *,Slot>::iterator it2 = ret.first; it2 != ret.second; ++it2) {
            if ((*it2).second.task == nullptr)
              scheduler_counter[*it]++;
          }
        }

        // Takes the partition with the highest number of free slots

        scheduler_counter_higher = scheduler_counter.begin();

        for (map<Scheduler *,unsigned int>::iterator i = scheduler_counter.begin();
             i != scheduler_counter.end();
             ++i) {
          if ((*i).second > (*scheduler_counter_higher).second) {
            scheduler_counter_higher = i;
          }
        }

        // If no partition has free slots, takes the partition with lower "load", where
        // load = pending_tasks / number_of_slots

        if ((*scheduler_counter_higher).second == 0) {
          for (it = affinity.begin() ; it != affinity.end(); ++it) {
            if ((dynamic_cast<ClusteredScheduler *>(*it))->size() / number_of_slots(*it)
                < (dynamic_cast<ClusteredScheduler *>(*shortest))->size() / number_of_slots(*shortest))
              shortest = it;
          }
          return *shortest;
        } else {
          return (*scheduler_counter_higher).first;
        }

      default:
        return nullptr;
        break;
    }
  }


  void FPGAKernel::onArrival(AbsRTTask * t)
  {
    HardwareTask *hw = dynamic_cast<HardwareTask *>(t);

    ////////// TODO
    /// Inventare una policy per scegliere la coda FIFO
    /// piu` conveniente

    Scheduler *partition = dispatcher(hw->getAffinity());

    //unsigned int scheduler_index = 0;
    //while (!affinity & 1) {
    //  affinity = affinity >> 1;
    //  scheduler_index++;
    //}

    if (partition == nullptr)
      throw FPGAKernelExc("Wrong partition chosen for scheduling hardware task!");
    /////////////////

    multimap<Scheduler *, Slot>::iterator p = scheduler.find(partition);

    (*p).first->addTask(t, "");
    (*p).first->insert(t);

    dispatch();
  }


  void FPGAKernel::onEnd(AbsRTTask * t)
  {
    multimap<Scheduler *, Slot>::iterator it;

    for (it = scheduler.begin(); it!=scheduler.end(); ++it) {
      if ((*it).second.task == t) {
        (*it).second.task = nullptr;
        break;
      }
    }

    dispatch();
  }


  CPU *FPGAKernel::getProcessor(const AbsRTTask *t) const {
    DBGENTER(_KERNEL_DBG_LEV);

    multimap<Scheduler *, Slot>::const_iterator it;

    for (it = scheduler.begin(); it!=scheduler.end(); ++it)
      if ((*it).second.task == t)
        return (*it).second.cpu;

    return getOldProcessor(t);
  }


  CPU *FPGAKernel::getOldProcessor(const AbsRTTask *t) const {

    multimap<Scheduler *, Slot>::const_iterator it;

    for (it = scheduler.begin(); it!=scheduler.end(); ++it)
      if ((*it).second.task == t)
        return (*it).second.old_cpu;

    return nullptr;
  }

  bool FPGAKernel::requestResource(AbsRTTask *t, const string &r, int n)
  throw(FPGAKernelExc)
  {
    DBGENTER(_KERNEL_DBG_LEV);

    if (_resMng == nullptr)
      throw FPGAKernelExc("Resource Manager not set!");

    bool ret = _resMng->request(t,r,n);
    if (!ret) {
      dispatch();
    }
    return ret;
  }

  void FPGAKernel::releaseResource(AbsRTTask *t, const string &r, int n)
  throw(FPGAKernelExc)
  {
    if (_resMng == nullptr)
      throw FPGAKernelExc("Resource Manager not set!");

    _resMng->release(t,r,n);

    dispatch();
  }


}
