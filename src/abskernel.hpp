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
#ifndef __ABSKERNEL_HPP__
#define __ABSKERNEL_HPP__

#include <abstask.hpp>

namespace RTSim {

    class CPU;

    /**
       \ingroup kernel
  
       Abtract Kernel interface. It defines the minimal
       functionality that should be implemented by a kernel. A
       Kernel is a generic objects that handles tasks. See also
       the definition of abstract task.
    */
    class AbsKernel
    {
    public:
        /**
           Virtual destructor. It avoids a warning for the
           presence of virtual functions.
        */
        virtual ~AbsKernel() {}

        /**
           Inserts the task in the ready queue. It doesn't
           check if the task has to be scheduled and doesn't
           make any context switch */
        virtual void activate(AbsRTTask* t) = 0;

        /**
           Extract the task from the ready queue. Again, this
           function doesn't check if the task has to be
           descheduled. */
        virtual void suspend(AbsRTTask* t) = 0;

        /**
           Checks if the current executing task is equal to
           the newly scheduled task. If so, does
           nothing. Otherwise, descheduler the old executing
           task and schedule the newly executing task. */
        virtual void dispatch() = 0;

        /**
           Invoked when a task "arrives".  */
        virtual void onArrival(AbsRTTask* t) = 0;

        /**
           Invoked when a task ends.  */
        virtual void onEnd(AbsRTTask* t) = 0;

        /**
           Returns a pointer to the CPU on whitch t is running
           (NULL if t is not running on any CPU)
        */
        virtual CPU* getProcessor(const AbsRTTask*) const = 0;

        /**
           Returns a pointer to the CPU on whitch t was running before
           being suspended (NULL if t was not running on any CPU)
        */
        virtual CPU* getOldProcessor(const AbsRTTask*) const = 0;

        /* It returns the current speed of the CPU. */
        virtual double getSpeed() const = 0;
  
        /** It sets the speed of the CPU accordingly to the
            new system load, and returns the new speed.
        */
        virtual double setSpeed(double newLoad) = 0;

        /** ??? */
        virtual bool isContextSwitching() const = 0;

    };

} // namespace RTSim

#endif
