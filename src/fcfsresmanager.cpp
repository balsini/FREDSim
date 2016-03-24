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
/*
 * $Id: fcfsresmanager.cpp,v 1.2 2005/04/28 01:34:48 cesare Exp $
 *
 * $Log: fcfsresmanager.cpp,v $
 * Revision 1.2  2005/04/28 01:34:48  cesare
 * Moved to sstream. Headers install. Code cleaning.
 *
 * Revision 1.1  2004/12/01 02:08:18  cesare
 * *** empty log message ***
 *
 * Revision 1.3  2004/11/26 03:47:10  cesare
 * Finished merging the main trunk with Lipari's branch.
 *
 */
#include <abskernel.hpp>
#include <fcfsresmanager.hpp>
#include <resource.hpp>
#include <task.hpp>

#include <vector>

namespace RTSim {

    FCFSResManager::FCFSResManager(const string &n) : ResManager(n)
    {
    }
 
    void FCFSResManager::newRun() 
    {
    }

    void FCFSResManager::endRun() 
    {
      for (unsigned int i=0; i<_res.size(); ++i) {
        _resAndCurrUsers[_res.at(i)] = nullptr;
        while (!_blocked[_res.at(i)].empty())
          _blocked[_res.at(i)].pop_front();
      }
    }
 
    bool FCFSResManager::request(AbsRTTask *t, Resource *r, int n) 
    { 
        DBGENTER(_FCFS_RES_MAN_DBG_LEV);

        bool ret;

        if (r->isLocked()) {
            DBGPRINT("Suspending task ");  
            _kernel->suspend(t);
            _blocked[r].push_back(t); 
            ret = false;
        } 
        else {
            DBGPRINT("Locking resource");
            r->lock(t);
//             r->setOwner(t);
            _resAndCurrUsers[r] = t;
            ret = true;
        }

        
        return ret;
    }

    void FCFSResManager::release(AbsRTTask *t, Resource *r, int n) 
    { 
        DBGENTER(_FCFS_RES_MAN_DBG_LEV);

        DBGPRINT("Unlocking resource ");

        r->unlock();
        //r->setOwner(0);
        _resAndCurrUsers[r] = NULL;
        if (!_blocked[r].empty()) {
    
            DBGPRINT("Relocking resource");

            r->lock(_blocked[r].front());
            //r->setOwner(_blocked[r].front());
            _resAndCurrUsers[r] = _blocked[r].front();
            _kernel->activate(_blocked[r].front());
            _blocked[r].pop_front(); 
        }

        
    }

}
