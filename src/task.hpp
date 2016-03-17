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
#ifndef __TASK_HPP__
#define __TASK_HPP__

/* Headers from MetaSim */
#include <entity.hpp>
#include <gevent.hpp>
#include <randomvar.hpp>
#include <strtoken.hpp>
#include <trace.hpp>

/* Headers from RTLib */
#include <abstask.hpp>
#include <kernel.hpp>
#include <taskevt.hpp>
#include <feedback.hpp>
#include <taskexc.hpp>

#define _TASK_DBG_LEV "Task"

namespace RTSim {

    /* Forward declaration... */
    class Instr;
    class InstrExc;

    // Task states
    typedef enum { TSK_IDLE, TSK_READY, TSK_EXEC, TSK_BLOCKED } task_state;

    /** 
        \ingroup tasks

        This class models a cyclic task. A cyclic task is a task that is
        cyclically activated by a timer (for example a periodic task) or
        by an external event (sporadic or aperiodic task). 
      
        This class models a "run-to-completion" semantic.  Every
        activation (also called arrival), an instance of the task is
        executed. The task executes all the instructions in the
        sequence until the last one, and then the instance is
        completed (task end). At the next activation, the task starts
        executing a new instance, and the instruction pointer is reset
        to the beginning of the sequence.

        Also:

        - when a job arrives, the corresponding deadline is set
        (the class Task has no deadline parameter).
        
        - it adds a deadline event to check deadline misses; it can
        abort the simulation in case of deadline miss (depending on
        the abort parameter in the constructor).

    */
    class Task : public Entity, virtual public AbsRTTask {
    private:
	// Hide copy constructor and assignment, tasks cannot be copied
	Task(const Task &);
	Task & operator=(const Task &);

    protected:
	MetaSim::RandomVar *int_time;  // The task is owner of this varible
        MetaSim::Tick lastArrival;     // The arrival of the last instance!
        MetaSim::Tick phase;           // Initial phasing for first arrival
        MetaSim::Tick arrival;         // Arrival time of the current (last) instance
        MetaSim::Tick execdTime;       // Actual Real-Time execution of the task
        MetaSim::Tick _maxC;           // Maximum computation time 
	std::deque <MetaSim::Tick> arrQueue; // Arrival queue, sorted FIFO
        int arrQueueSize;      // -1 stands for no-limit

	task_state state;      // IDLE, READY, EXECUTING, BLOCKED 

        //bool active;           // true if the current request has not completed
        //bool executing;        // true if the task is currently executing

        typedef std::vector<Instr *> InstrList;
        typedef std::vector<Instr *>::iterator InstrIterator;
        typedef std::vector<Instr *>::const_iterator ConstInstrIterator;
        InstrList instrQueue;
        InstrIterator actInstr;

        AbsKernel *_kernel;

        MetaSim::Tick _lastSched;
        MetaSim::Tick _dl;
        MetaSim::Tick _rdl;

        AbstractFeedbackModule *feedback;

    public:
        // Events need to be public to avoid an excessive fat interface.
        // Rhis is especially true when considering the probing mechanism
        // (for statistical collection and tracing). 

        ArrEvt arrEvt;
        EndEvt endEvt;
        SchedEvt schedEvt;
        DeschedEvt deschedEvt;
        FakeArrEvt fakeArrEvt;
        KillEvt killEvt;

        DeadEvt deadEvt;
        
        /**
         Returns a constant reference to the instruction queue
         (instrQueue)
         */
        const InstrList& getInstrQueue() { return instrQueue; };
        
        /**
         Returns a constant reference to the actual instruction
         (actInstr)
         */
        const InstrIterator& getActInstr() {return actInstr;};
        
        /**
	   Reset the instruction queue pointer:
	   actInstr = instrQueue.begin()
         */
        void resetInstrQueue();

    protected:
        friend class ArrEvt;
        friend class EndEvt;
        friend class KillEvt;
        friend class SchedEvt;
        friend class DeschedEvt;
        friend class FakeArrEvt;
        friend class DlineSetEvt;
        friend class DeadEvt;

        /**
           This event handler is invoked every time an arrival event 
           is triggered. 
	*/
        virtual void onArrival(MetaSim::Event *);

        /**
           This event handler is invoked when a task completes an instance.
           It resets the executed time counter, and the instruction counter.
	   
           @todo change its name into onInstanceEnd().
        */
        virtual void onEndInstance(MetaSim::Event *);
        
        /**
	   This event handler is invoked when a task instance has been killed.
	   Similar to onEndInstance, but the endEvt is not processed. 
         */
        void onKill(MetaSim::Event *);

        /**
           This event handler is invoked everytime the task is scheduled (i.e. 
           dispatched by the kernel). 
        */
        void onSched(MetaSim::Event *);

        /**
           This event handler is invoked everytime the task is suspended 
           by the kernel or by another entity, or it suspends itself. 
        */
        void onDesched(MetaSim::Event *);

        /**
           This event handler is ivoked everytime a buffered arrival has 
           to be processed. In this case:
           - an arrival event was triggered while the task was still active
           and was buffered
           - the task has just completed an instance
           - the buffered arrival event is processed

           It is used maily for tracing reason (there is not explicit need
           for an event here).
        */
        void onFakeArrival(MetaSim::Event *);

        /** 
            Reactivates the task. This method is used to implement a
            cyclic task. When an arrival event is processed by the
            onArrival() event handler, this function is called to post the
            next arrival event. 

            @todo re-think this function to implement a different kind of
            task.
        */
        void reactivate();

        /** 
            Handle arrival. This is the true arrival event handler.

            @todo simplify the arrival handling, by reducing the number of
            methods to be invoked.
	*/
        virtual void handleArrival(Tick arrival);

        /** handles buffered arrivals:  inserts an arrival in the buffer */
        void buffArrival();

        /** handles buffered arrivals: removes an arrival from the buffer */
        void unbuffArrival();

        /** handles buffered arrivals: returns an arrival from the buffer */
        Tick getBuffArrival();

        /** handles buffered arrivals: returns true if there is a buffered
            arrival */
        bool chkBuffArrival() const;

	// blocking a task: 
	// I deschedule the task, then it goes into the blocking state. 
	// It can be unblocked only when an Unblock() is called

        /******************************************************************/
        
    public:

        /** Constructor.

            @param iat Random variable that models activation events. 
            If this parameter is set equal to NULL (or 0), 
            then the task is NOT cyclically activated. In other 
            words, there is no arrival event automatically posted, 
            but the task must be activated by another entity 
            (task or interrupt, or else).
            @param rdl  relative deadline of the task. It is the maximum
            time since activation the task has to finish its job.
            @param ph  initial time of the activation. It is the first time at which
            the arrival event is posted. This parameter is ignored when 
            iat = 0. 
            @param name Unique name for this entity. 

            @param qs Maximum size of the arrival buffer. This is the maximum 
            number of arrivals that can be buffered. By default, this is
            equal to __LONG_MAX__. If equal to zero, an arrival event is 
            discarded when the task is already active.

            @param maxC Worst-case execution time of an instance. This 
            parameter is not used for the simulation itself, but only 
            for some algorithm, or for analysis). See getWCET(). */

        Task(RandomVar *iat, Tick rdl, Tick ph = 0, 
             const std::string &name = "", 
             long qs = 1000, Tick maxC=0);

        /**
           Virtual destructor. Does nothing.

           @todo should it destroy the instructions? probably the answer is YES.
        */
        virtual ~Task();

        /**
           For the abstract factory
         */
        static Task* createInstance(vector<string> &par);

        /**
           Initializes the internal task structures at the beginning of each run.
           For example:
           - posts the first arrival event
           - resets the instruction counter
           - resets the executed time counter
        */
        virtual void newRun(); 

        /**
           Does nothing.
        */
        virtual void endRun();

        /** 
            This functions activates the tasks (post the arrival event at
            the current time).
        */
        virtual void activate();

        /** This method is used to activate the task (posts the arrival
            event at the time specified in the parameter) 
	
            @param t time of the arrival event

            @todo what if the task is active? 
            what if the time is in the past?
            what if the arrival event is already posted?
        */
        virtual void activate(Tick t);

        /** 
            This method permits to kill a task instance that is currently
            executing. The instruction pointer is reset to the first
            instruction, and the executed time counter is reset to 0. No
            further event is posted (in particular, the TaskEndEvt is not
            posted).

            @throws TaskNotActive if the task is not currently active
            @throws TaskNotExecuting if the task is not executing
	
            @todo the last throws is quite suspect....
        */ 
        void killInstance() throw(TaskNotActive, TaskNotExecuting);

        
        /**
            This method permits to select the behaviour of the task when a 
            deadline miss occurs.
         
            @param kill = true, to kill the task when a deadline miss occurs
            @param kill = false, to contine the task when a deadline miss occurs
         */
        void killOnMiss(bool kill);
        
        /** 
            Event propagated by instructions. It is invoked when an
            instruction is completed. The default behavior is to increment
            the instruction pointer to the next instrution in the list. If
            there are no more instrutions, the TaskEndEvt is posted at the
            current time.
        */
        virtual void onInstrEnd();


	void block();
	void unblock();


        /** 
            Specify that this task has to be traced 
        */
        virtual void setTrace(Trace *t);

        /** 
            Adds a new instruction at the end of the instruction list. This
            method has to be invoked during initialization to specify the
            list of instructions for a task.

            The instructions created and added in this way must be
            destroyed by the Task destructor. Therefore, the owner of the
            object pointed by instr is the task.
        */
        void addInstr(Instr *instr);

        /** 
            Removes an instruction from the instruction list. This method
            is invoked mainly during destruction of the task.
        */
        //    void removeInstr(Instr *instr) throw(NoSuchInstr);

        /** 
            Removes all instructions. This method removes all instructions
            from the instruction list. 

            @param selfDestruct when this is true (by default is false),
            the insctructions are destroyed (deallocated). This method is
            invoked during the destruction of the task.
        */
        void discardInstrs(bool selfDestruct = true);

        /** Returns the arrival time of the current instance */
        Tick getArrival() const;

        /** Returns the arrival time of the previous instance */
        Tick getLastArrival() const;

        /** Returns the executed time of the last (or current) instance */
        Tick getExecTime() const;

	Tick getMinIAT() const { return Tick(int_time->getMinimum());}

        virtual Tick getLastSched() {return _lastSched;}

        /** 
            Change the interarrival time. Used to change the period or the
            arrival characteristics of the task.

            @param iat a random variable that describes the arrival
            pattern for this cyclic task.
	
            @return the pointer to the old variable random variable

            @todo what happens to the existing iat variable?  probably to
            be changed: the task is the owner of the variable, so it
            should be responsible for deletion. Therefore, we would simply
            return the period or minimum interarrival time, or just a copy
            of the random variable with another function. This function
            should not return anything.
        */
        RandomVar *changeIAT(RandomVar *iat);

        /** 
            From AbsTask interface...
	
            @todo check if it calls the schedEvt.process()...
        */
        virtual void schedule();

        /**
           From AbsTask interface...
	
           @todo check if it calls the deschedEvt.process()...
        */
        virtual void deschedule();

        /** 
            Set the kernel for this task. Called by kernel.addTask(). If
            you want to remove a task from a kernel, call this function
            with k=NULL.

            @param k pointer to the kernel or 0.

            @throws KernelAlreadySet if the user has already set a kernel
            for this task. To move a task from one kernel to another, call
            setKernel(0) before.
        */
        void setKernel(AbsKernel *k) throw(KernAlreadySet);

        /** 
            Returns the kernel that contains this task. Can return 0
            if this taks does not belong to any kernel.
        */
        AbsKernel *getKernel() { return _kernel; }

        /** 
            Returns true if the task is active.
        */
        virtual bool isActive() const;

        /** 
            Returns true if the task is executing.
        */
        virtual bool isExecuting() const;

        /** 
            Returns the worst-case computation time. If the maxC parameter
            in the constructor has not been set, or if it has been set
            equal to 0, then this function invokes the getWCET() on every
            instruction in the instruction list, and sums the number
            togheter.  If maxC has been set to a number different from 0,
            then this function returns maxC.
        */
        virtual Tick getWCET() const;

        Tick getDeadline() const {return _dl;}
        Tick getRelDline() const {return _rdl;}

        void setRelDline(const Tick& dl) {_rdl = dl;}

        /** 
            Returns a pointer to the CPU on which this task is executing.
        */
        virtual CPU *getCPU() const;

        /** 
            Returns a pointer to the old CPU on which this task was
            executing.
        */
        CPU *getOldCPU() const;

        /** 
            Parse and insert instructions into this task. The input string
            must be a sequence of instructions separated by a
            semicolon. Last instruction must also end with a
            semicolon. The instruction can be one of the following:

            - fixed(n) takes an integer number and generates an
            instruction with fixed computation time;
            - delay(var) takes a random variable and generates a ExecInstr
            instruction with variable execution time, according to the
            specified random variable;
            - wait(r) or lock(r) takes a string r containing the name of a
            resource and produces a WaitInstr on that resource;
            - signal(r) or unlock(r) takes a string r containing the name
            of a resource and produces a SignalInstr on that resource;
            - activate(t) takes a string containing the name of a task and
            generates an ActivateInstr on that task.

            An example of usage is the following:

            @code
            t1.insertCode("fixed(4);wait(Res1);delay(unif(4,10));
            signal(Res1); delay(unif(10,20));");
            @endcode

            In this case, the task performs 5 instructions; the first one
            lasts 4 ticks; the second one is a wait on resource Res1; the
            third one has variable execution time, uniformely distributed
            between 4 and 10 ticks; the fourth one is a signal on resource
            Res1; finally, the last instruction has variable execution
            time uniformely distributed between 10 and 20 ticks.
        */
        void insertCode(const string &code); //throw(ParseExc);

        /**
           Sets the feedback module for this task (optional, by
           default no feedback is needed).
         */

        void setFeedbackModule(AbstractFeedbackModule *afm);

        /** 
            For debugging reasons. Normally you do not need to call this!

            @todo to be removed eventually.
        */ 
        void printInstrList() const;

        /** 
            Function inherited from AbsTask. It refreshes the state of the
            executing task when a change of the CPU speed occurs.

            @todo check which function calls this one.
        */ 
        void refreshExec(double oldSpeed, double newSpeed);

        int getTaskNumber() const { return getID();}	

        void setAbort(bool f) { deadEvt.setAbort(f); }
    };

    /// returns the task name, or "(nil)" if the pointer does not point 
    /// to a task entity
    std::string taskname(const AbsRTTask *t);

} // namespace RTSim

#endif
