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
#ifndef __CLUSTEREDSCHED_HPP__
#define __CLUSTEREDSCHED_HPP__

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
    class ClusteredScheduler : public Scheduler {
        class ClusteredModel : public TaskModel {
        public:
            ClusteredModel(AbsRTTask *t) : TaskModel(t) {}
            Tick getPriority() { return _rtTask->getArrival(); }
            void changePriority(MetaSim::Tick)
                {
                    cerr << "Warning! changePriority called on a ClusteredModel" << endl;
                }
        };

    public:
        /**
           Create an ClusteredModel, passing the task. It throws a
           RTSchedExc exception if the task is already present
           in this scheduler.
        */
        void addTask(AbsRTTask *t) throw(RTSchedExc);

        virtual ~ClusteredScheduler() {
            for (auto it=_tasks.begin(); it!=_tasks.end(); ++it)
                delete it->second;
        }

        void addTask(AbsRTTask *t, const std::string &p);

        void removeTask(AbsRTTask *t);

        unsigned int size();

        static ClusteredScheduler *createInstance(vector<string> &par);
    };


} // namespace RTSim 

#endif
