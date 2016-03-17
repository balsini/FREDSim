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
#ifndef __RESMANAGER_HPP__
#define __RESMANAGER_HPP__

#include <map>
#include <set>
#include <string>

#include <entity.hpp>
#include <scheduler.hpp>

#define _RESMAN_DBG_LEV  "ResMan"

namespace RTSim {

    class AbsRTTask;
    class Resource;
    class AbsKernel;

//   using namespace std;
    using namespace MetaSim;

    /** 
        \ingroup resman
        Generic resource manager. A specific resource manager should be derived 
        from this class.
      
        @todo: add simple documentation for this class.
      
        @see Resource
    */
    class ResManager : public Entity {
        friend class RTKernel;
        friend class FPGAKernel;
    public:
        /** Constructor of ResManager */
        ResManager(const std::string &n = "");
  
        virtual ~ResManager();

        /**
           Adds the resource to the set of resources managed by the Resource
           Manager.  should check if the resource is already present in such
           set 
       
           @param name resource name;
           @param n number of unit (for supporting multi-unit resources), 
           by default is 1.
        */
        virtual void addResource(const std::string &name, int n=1);

        /**
         * Function called by a task instr (the WaitInstr) to perform an
         * access request to a specific resource. That access could be
         * granted or the task could be suspended. It returns true if the
         * resource has been locked succesfully; it returns false if the
         * task has been blocked.

         * @todo: Should check if the resource is among the ones handled by
         * this manager

         * @param t task
         * @param name resource name
         * @param n number of units (by default is 1).
         */
        bool request(AbsRTTask *t, const std::string &name, int n=1);

        /**
         * Function called by a task instr to perform the release of a
         * specific resource. The consequence of this call could be the
         * reactivation of one or more suspended tasks.  
   
         * @todo Should check if the resource is among the ones handled by
         * this manager, and was locked before

         * @param t task
         * @param name resource name
         * @param n number of units (by default is 1).
         */
        void release(AbsRTTask *t, const std::string &name, int n=1);

        /*
         * Function called to specify that task t uses the resource called
         * name. This function is not necessary in simple resource managers,
         * like FCFSResManager. It is useful for PCRManager, for computing
         * the ceilings! 

         * @todo Maybe, it should be moved in PCR, then!!
         *
         * @see PCRManager
         */
        // virtual void addUser(AbsRTTask *t, const std::string &name, int n=1) = 0;

    protected:

        AbsKernel *_kernel;
        Scheduler *_sched;

        /**
         * Set the kernel and the associated scheduler. Do not call
         * this function from your main program!! This function is
         * automatically called by the RTKernel::setResManager().
         *
         * @see RTKernel
         */
        void setKernel(AbsKernel *k, Scheduler *s);

        std::vector<Resource *> _res;

        virtual bool request(AbsRTTask *t, Resource *r, int n=1) = 0;
        virtual void release(AbsRTTask *t, Resource *r, int n=1) = 0;
    };
} // namespace RTSim 

#endif
