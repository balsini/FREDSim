/***************************************************************************
 begin                : Thu Apr 24 15:54:58 CEST 2003
 copyright            : (C) 2003 by Giuseppe Lipari
 email                : lipari@sssup.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cstdlib>
#include <cstring>

#include <regvar.hpp>
#include <factory.hpp>
#include <simul.hpp>
#include <strtoken.hpp>

#include <abskernel.hpp>
#include <instr.hpp>
#include <task.hpp>

namespace RTSim {

  using namespace std;
  using namespace MetaSim;
  using namespace parse_util;

  Task::~Task()
  {
    DBGENTER(_TASK_DBG_LEV);
    DBGPRINT("Destructor of class Task");
    discardInstrs(true);
  }

  Task::Task(RandomVar *iat, Tick rdl, Tick ph,
             const std::string &name, long qs, Tick maxC)
    : Entity(name),
      int_time(iat), lastArrival(0), phase(ph),
      arrival(0), execdTime(0), _maxC(maxC),
      arrQueue(), arrQueueSize(qs),
      meanRT(nullptr),
      maxRT(nullptr),
      minRT(nullptr),
      state(TSK_IDLE),
      instrQueue(),
      actInstr(),
      _kernel(NULL),
      _lastSched(0),
      _dl(0), _rdl(rdl),
      feedback(NULL),
      arrEvt(this), endEvt(this), schedEvt(this),
      deschedEvt(this), fakeArrEvt(this), killEvt(this),
      deadEvt(this, false, false)
  {
  }

  void Task::newRun(void)
  {
    if (!instrQueue.empty()) {
      actInstr = instrQueue.begin();
    } else throw EmptyTask();

    state = TSK_IDLE;
    while (chkBuffArrival()) unbuffArrival();

    lastArrival = arrival = phase;
    if (int_time != NULL) arrEvt.post(arrival);
    _dl = 0;
  }

  void Task::endRun(void)
  {
    while (!arrQueue.empty()) {
      arrQueue.pop_front();
    }
    arrEvt.drop();
    endEvt.drop();
    schedEvt.drop();
    deschedEvt.drop();
    fakeArrEvt.drop();
    deadEvt.drop();
  }

  /* Methods from the interface... */
  bool Task::isActive(void) const
  {
    return state != TSK_IDLE;
  }

  bool Task::isExecuting(void) const
  {
    return state == TSK_EXEC;
  };

  void Task::schedule(void)
  {
    DBGENTER(_TASK_DBG_LEV);
    DBGPRINT("Scheduling " << getName());

    schedEvt.process();
  }

  void Task::deschedule()
  {
    DBGENTER(_TASK_DBG_LEV);
    DBGPRINT("Descheduling " << getName());

    schedEvt.drop();

    deschedEvt.process();
  }

  void Task::setKernel(AbsKernel *k) throw(KernAlreadySet)
  {
    DBGENTER(_TASK_DBG_LEV);

    if (_kernel != nullptr) throw KernAlreadySet();

    _kernel = k;
  }

  void Task::setTrace(Trace *t)
  {
    // For the moment, let's remove this...
    // Propagate tracing over Pseudo Instructions

    InstrIterator i = instrQueue.begin();
    while (i != instrQueue.end()) {
      (*i)->setTrace(t);
      i++;
    }

    arrEvt.addTrace(t);
    fakeArrEvt.addTrace(t);
    endEvt.addTrace(t);
    schedEvt.addTrace(t);
    deschedEvt.addTrace(t);
  }

  void Task::reactivate()
  {
    Tick v;

    if (int_time != NULL) {
      v = (Tick) int_time->get();
      if (v > 0) arrEvt.post(SIMUL.getTime() + v);
    }
  }

  void Task::handleArrival(Tick arr)
  {
    DBGENTER(_TASK_DBG_LEV);

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
    if (_dl >= SIMUL.getTime()) deadEvt.post(_dl);

  }

  void Task::block()
  {
    // check that the task is not idle and is not already blocked
    if (state == TSK_IDLE or state == TSK_BLOCKED)
      throw string("Task cannot be blocked, because it is ") +
        (state == TSK_IDLE ? "idle" : "blocked");
    _kernel->suspend(this);
    state = TSK_BLOCKED;
    _kernel->dispatch();
  }

  void Task::unblock()
  {
    state = TSK_READY;
    _kernel->onArrival(this);
  }

  Tick Task::getArrival() const
  {
    return arrival;
  }

  Tick Task::getLastArrival() const
  {
    return lastArrival;
  }

  Tick Task::getExecTime() const
  {
    if (isActive()) {
      return execdTime + (*actInstr)->getExecTime();
    } else {
      return execdTime;
    }
  }

  Tick Task::getBuffArrival()
  {
    Tick time = arrQueue.front();

    arrQueue.pop_front();

    return time;
  }

  bool Task::chkBuffArrival() const
  {
    return !arrQueue.empty();
  }

  void Task::buffArrival()
  {
    if ((int)arrQueue.size() <= arrQueueSize) {
      arrQueue.push_back(SIMUL.getTime());
    }
  }

  void Task::unbuffArrival()
  {

    if (!arrQueue.empty()) {
      arrQueue.pop_back();
    }
  }

  RandomVar *Task::changeIAT(RandomVar *iat)
  {
    RandomVar *ret = int_time;

    int_time = iat;
    return ret;
  }

  void Task::addInstr(Instr *instr)
  {
    instrQueue.push_back(instr);
    DBGTAG(_TASK_DBG_LEV, "Task::addInstr() : Instruction added");
  }

  void Task::discardInstrs(bool selfDestruct)
  {
    // Unset all the Instructions
    if (selfDestruct) {
      ConstInstrIterator p = instrQueue.begin();

      while (p != instrQueue.end()) {
        delete *p;
        p++;
      }
    }
    // delete all list entries
    instrQueue.clear();
  }

  /* And finally, the event handlers!!! */

  void Task::onArrival(Event *e)
  {
    DBGENTER(_TASK_DBG_LEV);

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

  void Task::onEndInstance(Event *e)
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

  void Task::killInstance() throw(TaskNotActive, TaskNotExecuting)
  {

    DBGENTER(_TASK_DBG_LEV);

    if (not isActive()) {
      DBGPRINT("not active...");
      throw TaskNotActive("killInstance() on a non-active task");
    }

    endEvt.drop();

    (*actInstr)->deschedule();
    execdTime += (*actInstr)->getExecTime();

    if (chkBuffArrival()) {
      fakeArrEvt.post(SIMUL.getTime());
      DBGPRINT("[Fake Arrival generated]");
    }

    state = TSK_IDLE;

    killEvt.process();
  }

  void Task::onKill(Event *e)
  {
    DBGENTER(_TASK_DBG_LEV);

    // from old Task ...
    killEvt.drop();
    // normal code

    lastArrival = arrival;

    int cpu_index = getCPU()->getIndex();

    DBGPRINT("Task " << getName() << " killed on CPU "
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
  }

  void Task::onSched(Event *e)
  {
    DBGENTER(_TASK_DBG_LEV);

    int cpu_index = getCPU()->getIndex();

    DBGPRINT("schedEvt for task " << getName()
             << " on CPU " << cpu_index);

    if (not isActive()) {
      throw TaskNotActive("OnSched on a non-active task: " + getName());
    }
    if (isExecuting()) {
      throw TaskAlreadyExecuting();
    }

    schedEvt.setCPU(cpu_index);
    deschedEvt.drop();

    state = TSK_EXEC;

    (*actInstr)->schedule();

    // from Task ...
    deadEvt.setCPU(cpu_index);
  }

  void Task::onDesched(Event *e)
  {
    DBGENTER(_TASK_DBG_LEV);

    int cpu_index = getOldCPU()->getIndex();

    DBGPRINT("DeschedEvt for task " << getName()
             << "from CPU" << cpu_index);

    if (not isActive()) {
      throw TaskNotActive("OnDesched on a non-active task");
    }
    if (not isExecuting()) {
      throw TaskNotExecuting("OnDesched() on a non-executing task");
    }

    DBGPRINT_2("CPU: ", getCPU());
    deschedEvt.setCPU(cpu_index);
    endEvt.drop();

    (*actInstr)->deschedule();

    state = TSK_READY;
  }

  void Task::onInstrEnd()
  {
    DBGENTER(_TASK_DBG_LEV);
    DBGPRINT("task : " << getName());
    if (not isActive()) {
      DBGPRINT("not active...");
      throw TaskNotActive("onInstrEnd() on a non-active task");
    }
    // this exception conflicts with the implementation of suspendInstr.
    // I am removing it for the moment
    // if (not isExecuting()) {
    //     DBGPRINT("not executing...");
    //     throw TaskNotExecuting("OnInstrEnd() on a non executing task");
    // }
    execdTime += (*actInstr)->getExecTime();
    actInstr++;
    if (actInstr == instrQueue.end()) {
      DBGPRINT("End of instruction list");
      endEvt.post(SIMUL.getTime());
    } else if (isExecuting()) {
      (*actInstr)->schedule();
      DBGPRINT("Next instr scheduled");
    }
  }

  void Task::onFakeArrival(Event *e)
  {

    DBGENTER(_TASK_DBG_LEV);
    DBGPRINT_2("fakeArrEvt for task", getName());

    handleArrival(getBuffArrival());

    _kernel->onArrival(this);
  }

  void Task::activate()
  {
    arrEvt.drop();
    arrEvt.post(SIMUL.getTime());
  }

  void Task::activate(Tick t)
  {
    arrEvt.drop();
    arrEvt.post(t);
  }

  Tick Task::getWCET() const
  {
    Tick tt = 0;
    if (_maxC == 0) {
      ConstInstrIterator i = instrQueue.begin();
      while (i != instrQueue.end()) {
        tt += (*i)->getWCET();
        i++;
      }
    } else tt = _maxC;
    return tt;
  }

  void Task::insertCode(const string &code) //throw(ParseExc)
  {
    DBGENTER(_TASK_DBG_LEV);

    vector<string> instr = split_instr(code);

    for (unsigned int i=0; i<instr.size(); ++i) {
      vector<string>::iterator j;

      string token = get_token(instr[i]);
      string param = get_param(instr[i]);
      vector<string> par_list = split_param(param);

      par_list.push_back(string(getName()));


      for (j=par_list.begin();j!=par_list.end(); ++j)
        DBGPRINT_2(" - ", *j);
      DBGPRINT("");

      auto_ptr<Instr> curr = genericFactory<Instr>::instance().create(token, par_list);

      Instr *curr_instr = curr.release();

      if (!curr_instr) throw ParseExc("insertCode", token);

      DBGPRINT("Instr " << curr_instr->getName() << "  created.");

      addInstr(curr_instr);

      printInstrList();
    }


  }

  void Task::printInstrList() const
  {
    unsigned int i;

    DBGPRINT("Task " << getName() << ": instruction list");
    for (i=0; i<instrQueue.size(); ++i) {
      DBGPRINT(i << ") " << instrQueue[i]->getName());
    }
  }

  CPU *Task::getCPU() const
  {
    DBGTAG(_TASK_DBG_LEV, "Task::getCPU()");

    return _kernel->getProcessor(this);
  }


  CPU *Task::getOldCPU() const
  {
    DBGTAG(_TASK_DBG_LEV, "Task::getOldCPU()");

    return _kernel->getOldProcessor(this);
  }

  void Task::refreshExec(double oldSpeed, double newSpeed)
  {
    DBGENTER(_TASK_DBG_LEV);
    (*actInstr)->refreshExec(oldSpeed, newSpeed);

  }

  std::string taskname(const AbsRTTask *t)
  {
    const Entity *e = dynamic_cast<const Entity *>(t);
    if (e) return string(e->getName());
    else return "(nil)";
  }

  Task* Task::createInstance(vector<string> &par)
  {
    RandomVar* i = NULL;
    if (strcmp(par[0].c_str(), "0")) i = parsevar(par[0]);
    Tick d = Tick(par[1]);
    Tick p = Tick(par[2]);
    const char* n = "";
    if (par.size() > 2) n = par[3].c_str();
    long q = 1000;
    if (par.size() > 4) q = 1000;//atoi(par[4].c_str());
    bool a = true;
    if (par.size() > 5 && !strcmp(par[5].c_str(), "false")) a = false;
    Task* t = new Task(i, d, p, n, q, a);
    return t;
  }

  void Task::setFeedbackModule(AbstractFeedbackModule *afm)
  {
    feedback = afm;
  }

  void Task::resetInstrQueue()
  {
    actInstr = instrQueue.begin();
  }


  void Task::killOnMiss(bool kill)
  {
    deadEvt.setKill(kill);
  }

}
