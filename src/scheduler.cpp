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
#include <simul.hpp>

#include <scheduler.hpp>
#include <task.hpp>
#include <climits>

namespace RTSim {

    using namespace std;
    using namespace MetaSim;

    TaskModel::TaskModel(AbsRTTask* t)
        : _rtTask(t), active(false), 
         _insertTime(0), _threshold(INT_MAX) 
    {
    }

    TaskModel::~TaskModel()
    {
    }

    bool TaskModel::TaskModelCmp::operator()(TaskModel* a, TaskModel* b)
    {
        if (a->getPriority() < b->getPriority()) return true;
        else if (a->getPriority() == b->getPriority()) {
            if (a->getInsertTime() < b->getInsertTime()) return true;
            else if (a->getInsertTime() == b->getInsertTime())
                if (a->getTaskNumber() < b->getTaskNumber()) return true;
        }
        return false;
    }

    void TaskModel::setActive()
    {
        active = true;
    }

    void TaskModel::setInactive()
    {
        active = false;
    }

    bool TaskModel::isActive()
    {
        return active;
    }

/*-----------------------------------------------------------------*/

    Scheduler::Scheduler(): Entity(""), _kernel(0), _queue(), _tasks(), _currExe(0)
    {
    }

    Scheduler::~Scheduler() {}


    void Scheduler::enqueueModel(TaskModel* model)
    {
        AbsRTTask* task = model->getTask();

        if (find(task) != NULL)
          throw RTSchedExc("Element already present");
	
        _tasks[task] = model;
    }

    TaskModel* Scheduler::find(AbsRTTask* task)
    {
        map<AbsRTTask*, TaskModel*>::iterator mi = _tasks.find(task);
	
        if (mi == _tasks.end()) 
            return NULL;
        else return (*mi).second;
    }

    void Scheduler::setKernel(AbsKernel* k)
    {
        _kernel = k;
    }

    void Scheduler::insert(AbsRTTask* task) throw(RTSchedExc, BaseExc)
    {
        DBGENTER("Scheduler");

        TaskModel* model = find(task);
	
        if (model == NULL) {
            cerr << "Scheduler::insert Task model not found" << endl;
            cerr << "For task " << taskname(task) << endl;
            cerr << "Scheduler " << getName() << endl;
            throw RTSchedExc("AbsRTTaskNotFound");
        }
  
        model->setInsertTime(SIMUL.getTime());
        model->setActive();

        _queue.insert(model);

        
    }

    void Scheduler::extract(AbsRTTask* task) throw(RTSchedExc, BaseExc)
    {
        TaskModel* model = find(task);
	
        if (model == NULL) // raise an exception
            throw RTSchedExc("AbsRTTask not found");
		
        _queue.erase(model);
        model->setInactive();
    }

    int Scheduler::getPriority(AbsRTTask* task) throw(RTSchedExc)
    {
        TaskModel* model = find(task);
	
        if (model == NULL)
            throw RTSchedExc("AbsRTTask not found");
		
        return model->getPriority();
    }

    void Scheduler::changePriority(AbsRTTask* task, const std::string &params) 
        throw(RTSchedExc)
    {
        
    }


    int Scheduler::getThreshold(AbsRTTask* task) throw(RTSchedExc)
    { 
	TaskModel* model = find(task);
	
        if (model == NULL)
            throw RTSchedExc("AbsRTTask not found");
		
        return model->getThreshold();
    } 
 
    void Scheduler::setThreshold(AbsRTTask *task, const int th) throw(RTSchedExc)
    {
	TaskModel* model = find(task);
	
        if (model == NULL)
            throw RTSchedExc("AbsRTTask not found");
		
        model->setThreshold(th);
    }

    int Scheduler::enableThreshold(AbsRTTask* task)
	 throw(RTSchedExc)
    {
        TaskModel* model = find(task);
	
        if (model == NULL)
            throw RTSchedExc("AbsRTTask not found");

//      if (oldPriorities.find(task) == oldPriorities.end())
        oldPriorities[task] = model->getPriority();
//      else 
//          oldPriorities[t] = model->getPriority(); 

	extract(task);

	int tmp = model->getThreshold();

        //TODO: add some logic to avoid using or minimize threshold  

	model->changePriority(tmp);
	
	insert(task);
	
	return tmp;
    }

    void Scheduler::disableThreshold(AbsRTTask *task) throw(RTSchedExc)
    {
	//TODO: remove the old priority from the oldPriorities map	
        TaskModel* model = find(task);
	
        if (model == NULL)
            throw RTSchedExc("AbsRTTask not found");

	extract(task);

	model->changePriority(model->getPriority());
	
	insert(task);

	_kernel->dispatch();
    }

    void Scheduler::discardTasks(bool f)
    {
        typedef map<AbsRTTask*, TaskModel*>::iterator IT;

        _queue.clear();

        IT i = _tasks.begin();

        if (f) {
            while (i != _tasks.end()) {
                delete ((*i).second);
                i++;
            }
        }

        _tasks.clear();
    }

    AbsRTTask* Scheduler::getTaskN(unsigned int n)
    {
        DBGENTER("Kernel");

        if ( _queue.size() <= n ) {
            
            return NULL;
        }

        priority_list<TaskModel*, TaskModel::TaskModelCmp>::iterator it = 
            _queue.begin();
        for (unsigned int i = 0; i < n; i++) it++;

        

        return (*it)->getTask();    
    }

    void Scheduler::notify(AbsRTTask* task)
    {
        DBGENTER("Kernel");
	_currExe = task;
        
    }

    void Scheduler::newRun()
    {
        _queue.clear();

        typedef map<AbsRTTask*, TaskModel*>::iterator IT;

        for (IT i = _tasks.begin(); i != _tasks.end(); ++i) 
            i->second->setInactive();
    }

    void Scheduler::endRun()
    {
    }

    void Scheduler::print()
    {
        priority_list<TaskModel*, TaskModel::TaskModelCmp>::iterator it = 
            _queue.begin(); 

        DBGPRINT("Ready queue: ");
        for (; it != _queue.end(); ++it)
            DBGPRINT_2(taskname((*it)->getTask()), " -> ");
    }

    AbsRTTask* Scheduler::getFirst()
    {
        return getTaskN(0);
 
    }
}
