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
#ifndef __FPSCHED_HPP__
#define __FPSCHED_HPP__

#include <scheduler.hpp>

#define _FP_SCHED_DBG	"fpsched"

namespace RTSim {

    using namespace MetaSim;

#define _FP_SCHED_DBG_LEV "fpsched"

    /**
       \ingroup sched

       This class implements a generic fixed priority scheduler.

       This class redefines only the addTask function, because most of the
       work is done in the Scheduler class. The user must specify the
       priority of each task esplicitily.
    */
    class FPScheduler : public Scheduler
    {
    protected:

        class FPModel: public TaskModel
        {
        protected:
            Tick _prio;

        public:
            FPModel(AbsRTTask *t, Tick p) : TaskModel(t), _prio(p) {}
            Tick getPriority() { return _prio; }

            /// @todo: check the type
            void setPriority(Tick p) {_prio = p; }

            void changePriority(Tick p) {
                setPriority(p);
            }
        };


    public:
        /**
           Empty definition of pure virtual function addTask.

           @param t task to be added.
        */
        void addTask(AbsRTTask *t) throw(RTSchedExc) {}

        virtual ~FPScheduler() {
            for (auto it=_tasks.begin(); it!=_tasks.end(); ++it)
                delete it->second;
        }

        /**
           Create an FPModel passing the task and the priority. It throws a
           RTSchedExc exception if the task is already present in this
           scheduler.

           @param t task to be added.
           @param prio task's priority.
        */
        void addTask(AbsRTTask *t, int prio) throw(RTSchedExc);

        void addTask(AbsRTTask *t, const std::string &p);

        void removeTask(AbsRTTask *t) {}
                        
        static FPScheduler *createInstance(vector<string> &par);

    };

} // namespace RTSim 

#endif
