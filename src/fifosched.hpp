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
#ifndef __FIFOSCHED_HPP__
#define __FIFOSCHED_HPP__

#include <scheduler.hpp>
#include <algorithm>

namespace RTSim {
    /**
       \ingroup kernels

       This class implements a simple First in First out scheduling
       policy. In other words, tasks are order by their arrival time. It
       redefines only the addTask function, because most of the work is
       done in the Scheduler class.
    */
    class FIFOScheduler : public Scheduler {
        class FIFOModel : public TaskModel {
        public:
            FIFOModel(AbsRTTask *t) : TaskModel(t) {}
            Tick getPriority() { return _rtTask->getArrival(); }
            void changePriority(MetaSim::Tick)
                {
                    cerr << "Warning! changePriority called on a FIFOModel" << endl;
                }
        };

    public:
        /**
           Create an FIFOModel, passing the task. It throws a
           RTSchedExc exception if the task is already present
           in this scheduler.
        */
        void addTask(AbsRTTask *t) throw(RTSchedExc);

        void addTask(AbsRTTask *t, const std::string &p);

        void removeTask(AbsRTTask *t)
        {
          _tasks.erase(t);
        }

        static FIFOScheduler *createInstance(vector<string> &par);
    };


} // namespace RTSim 

#endif
