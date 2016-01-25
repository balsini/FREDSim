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
#ifndef __ACCELERATEDTASK_HPP__
#define __ACCELERATEDTASK_HPP__

#include <cstdlib>
#include <cstring>

#include <regvar.hpp>
#include <simul.hpp>

#include <task.hpp>
#include <hardwaretask.hpp>
#include <memory>

namespace RTSim {

    using namespace std;
    using namespace MetaSim;

    /**
       Models a simple accelerated task. It's a simpler interface to
       Task.
    */
    class AcceleratedTask: public Task
    {
        unique_ptr<HardwareTask> ht;
        Tick period;
    public:
        AcceleratedTask(Tick iat, Tick rdl, Tick ph = 0,
                     const std::string &name = "", long qs = 100);

        inline Tick getPeriod() { return period; }

        /** Used to build tasks with the Factory.  The string
            must contain a set of comma separated values, in
            the same order as in the constructor:

            - period, deadline, phase

            Then a set of optional paramters can be given: -
            name (a string), queuesize, abort (true/false)

            Please take into account that at least 3 arguments of
            numerical type must be given!

            Example:
            - PeriodiTask *p = AcceleratedTask::createInstance("10, 10, 0, task1");

            is the same as

            - AcceleratedTask *p = new AcceleratedTask(10, 10, 0, "task1");

        */
        Task * getHW() { return ht.get(); }
        static AcceleratedTask* createInstance(vector<string>& par);
    };

} // namespace RTSim 

#endif
