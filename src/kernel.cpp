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
#include <algorithm>

#include <simul.hpp>

#include <cpu.hpp>
#include <kernel.hpp>
#include <resmanager.hpp>
#include <scheduler.hpp>
#include <task.hpp>
#include <reginstr.hpp>

namespace RTSim {

  using namespace std;

  RTKernel::RTKernel(Scheduler *s, const std::string& name, CPU* c)
    : Entity(name),
      _sched(s),
      _resMng(0),
      _cpu(),
      internalCpu(true),
      beginDispatchEvt(this),
      endDispatchEvt(this),
      _isContextSwitching(false),
      _contextSwitchDelay(0)
  {
    __reginstr_init();
    __regsched_init();
    __regtask_init();

    _currExe = NULL;

    DBGENTER(_KERNEL_DBG_LEV);

    /* In this constructor the user can decide to provide
           a particular CPU or not.  If a CPU is provided
           (i.e. c!=NULL), that particular CPU will be
           used. Otherwise, a new CPU without power saving
           functions is created (using the statement new), and
           a boolean variable (i.e. internalCpu) is set. This
           variable specifies if the CPU must be deleted in
           the kernel destructor.
        */
    if (c != NULL){
      _cpu = c;
      internalCpu = false;
    }
    else{
      // Creates a CPU without power saving:
      _cpu = new CPU;
    }

    s->setKernel(this);
  }

  RTKernel::~RTKernel()
  {
    DBGENTER(_KERNEL_DBG_LEV);
    DBGPRINT("Destructor of RTKernel");
    if (internalCpu == true){
      DBGPRINT("Deleting internal CPU in the kernel");
      delete _cpu;
    }

  }

  void RTKernel::addTask(AbsRTTask &t, const string &params)
  {
    t.setKernel(this);
    _handled.push_back(&t);
    _sched->addTask(&t, params);
  }

  void RTKernel::removeTask(AbsRTTask &t)
  {
    t.setKernel(0);
    _handled.erase(find(_handled.begin(), _handled.end(), &t));
    _sched->removeTask(&t);
  }

  CPU* RTKernel::getProcessor(const AbsRTTask* t) const
  {
    return _cpu;
  }

  CPU* RTKernel::getOldProcessor(const AbsRTTask* t) const
  {
    return _cpu;
  }

  void RTKernel::activate(AbsRTTask *task)
  {
    DBGENTER(_KERNEL_DBG_LEV);

    _sched->insert(task);
  }

  AbsRTTask* RTKernel::getCurrExe() const
  {
    return _currExe;
  }

  void RTKernel::suspend(AbsRTTask *task)
  {
    DBGENTER(_KERNEL_DBG_LEV);

    _sched->extract(task);

    if (_currExe == task) {
      task->deschedule();
      _currExe = NULL;
    }
  }

  void RTKernel::discardTasks(bool f)
  {
    _sched->discardTasks(f);
    _handled.clear();
  }

  void RTKernel::onArrival(AbsRTTask *task)
  {
    DBGENTER(_KERNEL_DBG_LEV);
    DBGPRINT_2("Inserting ",
               taskname(task));

    _sched->insert(task);

    if(!_isContextSwitching){
      dispatch();
    } else {
      beginDispatchEvt.drop();
      beginDispatchEvt.post(endDispatchEvt.getTime());
    }
  }

  void RTKernel::onEnd(AbsRTTask *task)
  {
    DBGENTER(_KERNEL_DBG_LEV);

    if (getProcessor(task) == NULL) {
      throw RTKernelExc("Received a onEnd of a non executing task");
    }
    _sched->extract(task);
    _currExe = NULL;

    dispatch();
  }

  void RTKernel::dispatch()
  {
    DBGENTER(_KERNEL_DBG_LEV);

    // we have only to post an Dispatch event (low priority)

    beginDispatchEvt.drop();
    beginDispatchEvt.post(SIMUL.getTime());
  }

  void RTKernel::onBeginDispatch(Event* e)
  {
    DBGENTER(_KERNEL_DBG_LEV);

    AbsRTTask *newExe = _sched->getFirst();

    if (newExe != NULL)
      DBGPRINT_2("From sched: ",
                 taskname(newExe));

    if(_currExe != newExe){
      if (_currExe != NULL){
        _currExe->deschedule();
      }
      if( newExe != NULL) {
        _isContextSwitching = true;
        _currExe = newExe;
        endDispatchEvt.post(SIMUL.getTime() + _contextSwitchDelay);
      }
    } else {
      _sched->notify(newExe);
      if (newExe != NULL)
        DBGPRINT_2("Now Running: ",
                   taskname(newExe));
    }
  }


  void RTKernel::onEndDispatch(Event* e)
  {
    DBGENTER(_KERNEL_DBG_LEV);

    _currExe->schedule();

    DBGPRINT_2("Now Running: ",
               taskname(_currExe));

    _isContextSwitching = false;
    _sched->notify(_currExe);
  }


  void RTKernel::refresh()
  {
    DBGENTER(_KERNEL_DBG_LEV);
    dispatch();
  }

  void RTKernel::setResManager(ResManager* rm)
  {
    _resMng = rm;
    _resMng->setKernel(this, _sched);
  }

  bool RTKernel::requestResource(AbsRTTask *t, const string &r, int n)
  throw(RTKernelExc)
  {
    DBGENTER(_KERNEL_DBG_LEV);

    if (_resMng == 0) throw RTKernelExc("Resource Manager not set!");
    bool ret = _resMng->request(t,r,n);
    if (!ret)
      dispatch();
    return ret;
  }

  void RTKernel::releaseResource(AbsRTTask *t, const string &r, int n)
  throw(RTKernelExc)
  {
    if (_resMng == 0) throw RTKernelExc("Resource Manager not set!");

    _resMng->release(t,r,n);

    dispatch();
  }

  void RTKernel::setThreshold(const int th)
  {
    DBGENTER(_KERNEL_DBG_LEV);

    _sched->setThreshold(_currExe, th);
  }

  int RTKernel::enableThreshold()
  {
    DBGENTER(_KERNEL_DBG_LEV);

    int tmp = _sched->enableThreshold(_currExe);
    return tmp;
  }

  void RTKernel::disableThreshold()
  {
    DBGENTER(_KERNEL_DBG_LEV);

    _sched->disableThreshold(_currExe);
  }

  void RTKernel::printState() const
  {
  }

  void RTKernel::newRun()
  {
    _currExe = NULL;
  }

  void RTKernel::endRun()
  {
    _currExe = NULL;
  }

  void RTKernel::print() const
  {
  }

  std::vector<std::string>  RTKernel::getRunningTasks()
  {
    std::vector<std::string> tmp_ts;
    std::string tmp_name = taskname(_currExe);

    if (tmp_name != "(nil)")
      tmp_ts.push_back(tmp_name);

    return tmp_ts;
  }
}
