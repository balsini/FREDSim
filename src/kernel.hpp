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
#ifndef __KERNEL_HPP__
#define __KERNEL_HPP__

#include <deque>
#include <set>
#include <string>

#include <baseexc.hpp>
#include <entity.hpp>

#include <abskernel.hpp>
#include <kernevt.hpp>
#include <cpu.hpp>

#define _KERNEL_DBG_LEV "Kernel"

namespace RTSim {

    using namespace MetaSim;

    class Scheduler;
    class ResManager;

    /**
       \ingroup kernels

       Exception for the Kernel class
    */
    class RTKernelExc : public BaseExc {
    public:
        RTKernelExc(string msg) :BaseExc(msg, "RTKernel", "kernel.cpp") {}
    };

    /** 
        \ingroup kernels

        An implementation of a real-time single processor kernel.
        It contains :

        - a ponter to one CPU

        - a pointer to a Scheduler, which implements the
        scheduling policy;

        - a pointer to a Resource Manager, which is responsable
        for resource access related operations and thus implements
        a resource allocation policy;

        - the set of task handled by this kernel.
      
        This implementation is quite general: it lets the user of
        this class the freedom to adopt any scheduler derived form
        Scheduler and a resorce manager derived from ResManager
        or no resorce manager at all.

        It is up to the instruction class to implemet the correct
        duration of its execution by asking the kernel of its task
        the speed of the processor on whitch it's scheduled.

        We will probably have to derive from this class to
        implement static partition and mixed task allocation to
        CPU.
 
        @sa absCPUFactory, Scheduler, ResManager, AbsRTTask
    */
    class RTKernel : public Entity, public virtual AbsKernel {
    protected:

        /// The real-time scheduler
        Scheduler *_sched;
    
        /// The resource manager
        ResManager* _resMng;
    
        /// The currently executing task
        AbsRTTask * _currExe;
    
        /// The processor...
        CPU *_cpu;
    
        /// List of the tasks.
        deque<AbsRTTask *> _handled;

	bool _isContextSwitching;
    
	Tick  _contextSwitchDelay;

        /** 
            This boolean variable is true if _cpu was created
            using the command "new" in the constructor. It is
            used in the destructor to know if the delete
            should be done.
        */
        bool internalCpu;

	friend class DispatchEvt;
	friend class BeginDispatchEvt;
	friend class EndDispatchEvt;

    public:

	BeginDispatchEvt beginDispatchEvt;
	EndDispatchEvt endDispatchEvt;
    
        /**
           Constructor. It needs a pointer to the
           scheduler. Optionally, it is possible to specify
           the name of this entity and a CPU.  By default, the
           kernel will use a standard CPU object.

           @see CPU

           @param s      Real-Time scheduler to be used in the kernel
           @param name   Name of this kernel
           @param c      Pointer to a CPU object
        */
        RTKernel(Scheduler* s, const std::string &name = "", CPU* c= NULL); 

        /**
           Destructor of the class. It destroys the internal
           CPU object, unless it was passed as parameter in
           the constructor.
        */
        virtual ~RTKernel(); 

        /**
           This function simply inserts the task in the ready
           queue.  After this function, it is necessary to
           call the dispatch() to force a context switch
           between the tasks.

           @see dispatch
        */
        virtual void activate(AbsRTTask*);

        /**
           Removes the task from the ready queue. If the task
           was executing, the task is "descheduled", and
           pointer _currExe is set to NULL. Otherwise, the
           task is simply removed from the ready queue, and
           nothing happens to the executing task.
        */
        virtual void suspend(AbsRTTask*);

        /**
           Compares _currExe with the first task in the ready
           queue. If they are different, it forces a context
           switch. The corresponding schedule and deschedule
           functions of the two tasks are called.
        */
        virtual void dispatch();

	virtual void onBeginDispatch(Event* e);
	virtual void onEndDispatch(Event* e);

        /**
           This function is invoked from the task onArrival
           function, which in turn is invoked when a task
           arrival event is triggred. It inserts the task in
           the ready queue and calls the refresh method.

           @see refresh
        */
        virtual void onArrival(AbsRTTask*);

        /**
           This function is invoked from the task onEnd
           function, which in turn is invoked when a task
           completes the execution of the current instance. It
           removes the task from the ready queue, set _currExe
           pointer to NULL and invokes the refresh.

           @see refresh
        */
        virtual void onEnd(AbsRTTask*);
    
        /**
           Calls the refresh method of the scheduler to
           re-order the ready queue, and dispatch() to update
           the CPUs assignment to the tasks.

           @see dispatch, RTSim::Scheduler
        */
        virtual void refresh();
    
        /**
           Set the resource manager to be used for handling
           shared resource between tasks. The default is no
           resource manager, which means that an exception
           will be raised in case a task performs a wait or
           signal opration.
        */
        void setResManager(ResManager* rm);
    
        /**
           Forwards the request of resource r from task t to
           the resource manager. If the resource manager has
           not been set, a RTKernelExc exception is raised.
        */
        virtual bool requestResource(AbsRTTask *t, const string &r, int n=1)
            throw(RTKernelExc);
    
        /**
           Forwards the release of the resource r by task t to
           the resource manager. If the resource manager has
           not been set, a RTKernelExc is raised.
        */
        virtual void releaseResource(AbsRTTask *t, const string &r, int n=1)
            throw(RTKernelExc);
    

        /**
           Forwards the request of preemption threshold raising for 
	   the executing task to the scheduler.
        */

        virtual void setThreshold(const int th);

        virtual int enableThreshold();

        virtual void disableThreshold();

        /**
           The kernel needs a list of the tasks that are part
           of this kernel, for many different
           reason. Therefore, this function has to be
           called. The typical sequence of operations to
           create a running system is the following:
       
           - The scheduler is created;
                   
           - The kernel is created passing the address of the
           scheduler
                   
           - The task is created
                   
           - The task is added to the kernel, passing the
           proper scheduling parameters.
        */
        virtual void addTask(AbsRTTask &t, const string &param ="");
        virtual void removeTask(AbsRTTask &t);

        /**
           Returns a pointer to the CPU on which t is runnig
           (NULL if t is not running on any CPU). In this
           case, it will always return the same CPU, since we
           are on a single processor platform.
        */
        virtual CPU* getProcessor(const AbsRTTask* t) const;

        /**
           Returns a pointer to the CPU on which t was runnig last.
           (NULL if t has never run on any CPU). In this case, it will
           always return the same CPU, since we are on a single
           processor platform.
        */
        virtual CPU* getOldProcessor(const AbsRTTask* t) const;

        AbsRTTask* getCurrExe() const;

        /**
           Prints on the DEBUG stream the status of the kernel
           (the name of the task running on each
           processor). See the debugging features of metasim.
        */
        virtual void printState() const;

        /**
           Abstract kernels can be combined in a hierarchical
           way. As an example, a server implements the
           interface AbsKernel, and therefore it can be
           included in another parent kernel.  This is not the
           case of RTKernel object, which can never be put on
           top of another kernel. However, it can be root of
           the hierarchy.
       
           Since this object must be the end of the hierarchy,
           this function returns a pointer to itself.
        */
        //AbsKernel* getKernel() { return this; }

        virtual void discardTasks(bool);

        /**
           This function is common to all Entity
           objects. Before every new simulation run, this
           function is invoked. In this case, it just puts the
           _currExe pointer to NULL.
        */ 
        virtual void newRun();
    
        /**
           This function is common to all Entity
           objects. after every new simulation run, this
           function is invoked. In this case, it just puts the
           _currExe pointer to NULL.
        */   
        virtual void endRun();
    
        /**
           Prints the status of the objects on the DEBUG
           stream. In reality, this function does nothing! But
           it is virtual, so check the child classes.
       
           @todo check all child classes.
        */
        virtual void print() const;
  
        /**
           Function inherited from AbsKernel. It returns the
           current speed of the CPU.
        */
        double getSpeed() const {return (_cpu->getSpeed());}
  
        /**
           Function inherited from AbsKernel. It sets the
           speed of the CPU accordingly to the new system
           load, and returns the new speed.
        */
        double setSpeed(double newLoad) 
            {return (_cpu->setSpeed(newLoad));}

        /**
           Function inherited from AbsKernel. It says if the 
	   kernel is currently in context switch mode.
        */
        virtual bool isContextSwitching() const 
            { return _isContextSwitching; }

        /**
           Function to set the overhead of the context switching,
           that is, by default, zero.
        */
	virtual void setContextSwitchDelay( const Tick& t ) { 
            _contextSwitchDelay = t; 
        }

        /**
         It returns the name of the task (std::string) stored in a
         std::vector<std::string>.
         This solution is proposed to be compliant with the multi
         core kernel, for which there could be more than one task 
         running at the same time.
         */
        virtual std::vector<std::string> getRunningTasks();
    };
  
} // namespace RTSim 

#endif
