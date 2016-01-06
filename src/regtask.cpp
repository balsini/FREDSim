/***************************************************************************
    begin                : Thu Jun 21 12:36:01 CEST 2003
    copyright            : (C) 2003 by Cesare Bartolini
    email                : c.bartolini@isti.cnr.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <string>

#include <factory.hpp>
#include <AVRTask.hpp>
#include <task.hpp>
#include <rttask.hpp>
#include <acceleratedtask.hpp>
#include <reginstr.hpp>

namespace RTSim {

  using namespace std;

  const string TaskName("Task");
  const string PeriodicTaskName("PeriodicTask");
  const string AVRTaskName("AVRTask");
  const string AcceleratedTaskName("AcceleratedTask");
  
  /**
     This namespace should not be visible, and in any case, users
     should never access objects of this namefile. This is used just
     for initialization of the objects needed for the abstract
     factory that creates tasks.
   */
  namespace __task_stub
  {
    static registerInFactory<Task, Task, string>
    registerRT(TaskName);

    static registerInFactory<Task, PeriodicTask, string>
    registerPeriodic(PeriodicTaskName);

    static registerInFactory<Task, AcceleratedTask, string>
    registerAccelerated(AcceleratedTaskName);

    static registerInFactory<Task, AVRTask, string>
    registerAVR(AVRTaskName);
  } // namespace __var_stub

  void __regtask_init() {}

} // RTSim
