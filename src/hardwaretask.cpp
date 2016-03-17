#include "hardwaretask.hpp"
#include "acceleratedtask.hpp"
#include "instr.hpp"

#include <accelerate_instr.hpp>
#include <fpgakernel.hpp>

namespace RTSim {
  HardwareTask::HardwareTask(Tick rdl, Tick ph,
                             const std::string &name, long qs, Tick maxC)
    : Task(NULL, rdl, ph, name, qs, maxC)
  {
    meanRT = 0;
    maxRT = 0;
    minRT = 0;
    _configurationTime = 0;
    _affinity.clear();
  }

  HardwareTask* HardwareTask::createInstance(vector<string>& par)
  {
    Tick d = Tick(par[0]);
    Tick p = Tick(par[1]);
    const char* n = "";
    if (par.size() > 2) n = par[2].c_str();
    long q = 100;
    if (par.size() > 4) q = atoi(par[3].c_str());

    return new HardwareTask(d, p, n, q);
  }

  void HardwareTask::setParent(AcceleratedTask *p)
  {
    _parent = p;
  }

  void HardwareTask::onEndInstance(Event * e)
  {
    DBGENTER(_TASK_DBG_LEV);

    // from old Task ...
    deadEvt.drop();
    // normal code

    if (not isActive()) {
      throw TaskNotActive("OnEnd() on a non-active task");
    }
    if (not isExecuting()) {
      throw TaskNotExecuting("OnEnd() on a non-executing task");
    }

    actInstr = instrQueue.begin();
    lastArrival = arrival;

    int cpu_index = getCPU()->getIndex();

    DBGPRINT("Task " << getName() << " finished on CPU "
             << cpu_index);

    endEvt.setCPU(cpu_index);
    _kernel->onEnd(this);
    state = TSK_IDLE;

    //_parent->schedEvt.post(SIMUL.getTime());

    if (feedback) {
      DBGPRINT("Calling the feedback module");
      feedback->notify(getExecTime());
    }


    if (maxRT)
      maxRT->record(SIMUL.getTime() - lastArrival);
    if (meanRT)
      meanRT->record(SIMUL.getTime() - lastArrival);
    if (minRT)
      minRT->record(SIMUL.getTime() - lastArrival);

    _ai->resumeEvt.post(SIMUL.getTime());
  }

  void HardwareTask::setAccelerateInstr(AccelerateInstr * i)
  {
    _ai = i;
  }

  void HardwareTask::handleArrival(Tick arr)
  {
      DBGENTER(_TASK_DBG_LEV);

      lastArrival = SIMUL.getTime();

      if (isActive()) {
          DBGPRINT("Task::handleArrival() Task already active!");
          throw TaskAlreadyActive();
      }
      arrival = arr;
      execdTime = 0;
      actInstr = instrQueue.begin();

      // reset all instructions
      ConstInstrIterator p = instrQueue.begin();
      while (p != instrQueue.end()) {
          (*p)->reset();
          p++;
      }
  state = TSK_READY;
      _dl = getArrival() + _rdl;

  }

  CPU *HardwareTask::getCPU() const
  {
    DBGTAG(_TASK_DBG_LEV, "Task::getCPU()");

    return _cpu;
  }
}
