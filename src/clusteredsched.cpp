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
#include <clusteredsched.hpp>
#include <kernel.hpp>

namespace RTSim {

    void ClusteredScheduler::addTask(AbsRTTask *task) throw(RTSchedExc)
    {
        ClusteredModel *model = new ClusteredModel(task);

        if (find(task) != NULL) 
            throw RTSchedExc("Element already present");
	
        _tasks[task] = model;
    }


    void ClusteredScheduler::addTask(AbsRTTask* task, const std::string &p)
    {
        if (!dynamic_cast<AbsRTTask *>(task)) 
            throw RTSchedExc("Cannot add a AbsRTTask to Clustered");
        // ignoring parameters
        addTask(dynamic_cast<AbsRTTask *>(task));
    }


    void ClusteredScheduler::removeTask(AbsRTTask *t)
    {
      extract(t);
      map<AbsRTTask*, TaskModel*>::iterator it = _tasks.find(t);
      _tasks.erase(it);
    }


    unsigned int ClusteredScheduler::size()
    {
      return _tasks.size();
    }


    ClusteredScheduler * ClusteredScheduler::createInstance(vector<string> &par)
    {
        // todo: check the parameters (i.e. to set the default
        // time quantum)
        return new ClusteredScheduler;
    }
}
