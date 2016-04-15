#include "acceleratedtask.hpp"

namespace RTSim {
  AcceleratedTask::AcceleratedTask(Tick iat, Tick rdl, Tick ph,
                                   const std::string &name, long qs)
    : Task(new DeltaVar(iat), rdl, ph, name, qs), period(iat)
  {
    ht = std::unique_ptr<HardwareTask>(new HardwareTask(0, 0, "Hardware"+name));

    // The real code is inserted by the accelerate_instruction, this is just for
    // avoiding task exceptions.
    ht.get()->insertCode("fixed(0);");
  }

  AcceleratedTask* AcceleratedTask::createInstance(vector<string>& par)
  {
    Tick i = Tick(par[0]);
    Tick d = Tick(par[1]);
    Tick p = Tick(par[2]);
    const char* n = "";
    if (par.size() > 2) n = par[3].c_str();
    long q = 100;
    if (par.size() > 4) q = atoi(par[4].c_str());

    return new AcceleratedTask(i, d, p, n, q);
  }

  void AcceleratedTask::onArrival(Event *e)
  {
    DBGENTER(_TASK_DBG_LEV);

    lastArrival = SIMUL.getTime();

    if (!isActive()) {
      // Standard Task Arrival: do standard
      // book-keeping and forward the event to the
      // father

      handleArrival(SIMUL.getTime());
      _kernel->onArrival(this);
    } else {
      DBGPRINT("[Buffered]");
      // Buffered Task Arrival: enqueue the request
      // and generate a buffArrEvt for the father;
      // the event will be automatically deleted(),
      // since we put the disposable flag in post to
      // true
      // from old Task ...

      deadEvt.process();

      buffArrival();
    }
    reactivate();
  }

  void AcceleratedTask::onEndInstance(Event *e)
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

    if (feedback) {
      DBGPRINT("Calling the feedback module");
      feedback->notify(getExecTime());
    }

    DBGPRINT_4("chkBuffArrival for task ",
               dynamic_cast<Entity*>(this)->getName(),
               " = ",
               chkBuffArrival());

    if (chkBuffArrival()) {
      fakeArrEvt.process();

      DBGPRINT("[Fake Arrival generated]");
    }

    if (maxRT)
      maxRT->record(SIMUL.getTime() - lastArrival);
    if (meanRT)
      meanRT->record(SIMUL.getTime() - lastArrival);
    if (minRT)
      minRT->record(SIMUL.getTime() - lastArrival);
  }

}
