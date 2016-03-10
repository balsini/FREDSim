#include <algorithm>

#include <simul.hpp>

#include <cpu.hpp>
#include <fpgakernel.hpp>
#include <resmanager.hpp>
#include <scheduler.hpp>
#include <clusteredsched.hpp>
#include <task.hpp>

namespace RTSim {

  using namespace std;
  using namespace MetaSim;

  FPGAKernel::FPGAKernel()
  {
    _CPUFactory = new uniformCPUFactory();
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
  }


  void FPGAKernel::addTask(AbsRTTask &t, const string &params)
  {
    HardwareTask * h = dynamic_cast<HardwareTask *>(&t);
    h->setKernel(this);
  }


  void FPGAKernel::activate(AbsRTTask * t)
  {
    asm("NOP");
    asm("NOP");
    asm("NOP");
  }


  void FPGAKernel::suspend(AbsRTTask * t)
  {
    asm("NOP");
    asm("NOP");
    asm("NOP");
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


  void FPGAKernel::onArrival(AbsRTTask * t)
  {
    HardwareTask *hw = dynamic_cast<HardwareTask *>(t);

    ////////// TODO
    /// Inventare una policy per scegliere la coda FIFO
    /// piu` conveniente

    vector<Scheduler *> affinity = hw->getAffinity();
    Scheduler * partition = affinity.front();

    //unsigned int scheduler_index = 0;
    //while (!affinity & 1) {
    //  affinity = affinity >> 1;
    //  scheduler_index++;
    //}

    if (affinity.size() == 0)
      throw FPGAKernelExc("Task affinity is null!");
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

}
