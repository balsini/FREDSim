/***************************************************************************
    begin                : Tue Aug 02 18:07:04 CEST 2005
    copyright            : (C) 2005 by Cesare Bartolini
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
#ifndef __TASKEVT_HPP__
#define __TASKEVT_HPP__

#include <event.hpp>

#include <abstask.hpp>

namespace RTSim {

    using namespace MetaSim;

    class Task;

    /**
       \ingroup task

       This class is the base class for all task events.
    */
    class TaskEvt : public MetaSim::Event
    {
    protected:
        Task* _task;
        int _cpu;

    public:
        TaskEvt(Task* t, int p = _DEFAULT_PRIORITY) : 
            MetaSim::Event(p), _cpu(-1)  {_task = t;}
        Task* getTask() const {return _task;}
        void setTask(Task* t) {_task = t;}

        int getCPU() {return _cpu;}
        void setCPU(int cpu) {_cpu = cpu;}
    };

    /// arrival event for a task
    /// \ingroup task
    class ArrEvt: public TaskEvt
    {
    public:
        ArrEvt(Task* t) :TaskEvt(t) {}
        virtual void doit();

    };

    /// Block event for a task
    /// \ingroup task
    class BlockEvt: public TaskEvt
    {
    public:
        BlockEvt(Task* t) :TaskEvt(t) {}
        virtual void doit();

    };

    /// Unblock event for a task
    /// \ingroup task
    class UnblockEvt: public TaskEvt
    {
    public:
        UnblockEvt(Task* t) :TaskEvt(t) {}
        virtual void doit();

    };

    /// end of instance event
    /// \ingroup task
    class EndEvt: public TaskEvt
    {
    public:
        static const int _END_EVT_PRIORITY = _DEFAULT_PRIORITY - 2;
        EndEvt(Task* t) :TaskEvt(t, _END_EVT_PRIORITY) {}
        virtual void doit();
    };
    
    /// when a task is killed
    /// \ingroup task
    class KillEvt: public TaskEvt
    {
    public:
        static const int _END_EVT_PRIORITY = _DEFAULT_PRIORITY - 2;
        KillEvt(Task* t) :TaskEvt(t, _END_EVT_PRIORITY) {}
        virtual void doit();
    };

    /// when the event is triggered, the task starts executing on processor
    /// \ingroup task
    class SchedEvt: public TaskEvt
    {
    public:
        SchedEvt(Task* t) : TaskEvt(t) {}
        virtual void doit();
    };

    /// when this event is triggered, the task does not 
    /// execute anymore on the processor.
    /// \ingroup task
    class DeschedEvt: public TaskEvt
    {
    public:
        DeschedEvt(Task* t) :TaskEvt(t) {}
        virtual void doit();
    };

    /// to handle buffered arrivals
    /// \ingroup task
    class FakeArrEvt: public TaskEvt
    {
    public:
        FakeArrEvt(Task* t) :TaskEvt(t) { setPriority(_DEFAULT_PRIORITY - 1); }
        virtual void doit();
    };

    /** 
        \addtogroup tasks

        @{
    */
    class DlineSetEvt: public TaskEvt
    {
    protected:
        Tick _dline;

    public:
        DlineSetEvt(Task* t) :TaskEvt(t) {}
        virtual void doit() {}
        void setDline(Tick d) {_dline = d;}
        Tick getDline() {return _dline;}

    };

    class DeadEvt: public TaskEvt
    {
    protected:
        bool _abort;
        bool _kill;

    public:
        static const int _DEAD_EVT_PRIORITY = EndEvt::_END_EVT_PRIORITY + 3; 

        DeadEvt(Task* t, bool abort, bool kill)
            : TaskEvt(t, _DEAD_EVT_PRIORITY), _abort(abort), _kill(kill) {}

        virtual void doit();  
        void setAbort(bool f) {_abort = f;}
        void setKill(bool f) {_kill = f;}
    };

} // namespace RTSim

#endif
