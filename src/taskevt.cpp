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
 * $Id$
 *
 * $Log$
 */
#include <task.hpp>
#include <taskevt.hpp>
#include <cstdlib>

namespace RTSim {
    
    void ArrEvt::doit()
    {
        _task->onArrival(this);
    }
    
    void EndEvt::doit()
    {
        _task->onEndInstance(this);
    }
    
    void KillEvt::doit()
    {
        _task->onKill(this);
    }
    
    void SchedEvt::doit()
    {
        _task->onSched(this);
    }
    
    void DeschedEvt::doit()
    {
        _task->onDesched(this);
    }
    
    void FakeArrEvt::doit()
    {
        _task->onFakeArrival(this);
    }

    void BlockEvt::doit()
    {}

    void UnblockEvt::doit()
    {}

    void ConfigEvt::doit()
    {}

    void EndConfigEvt::doit()
    {}
    
    void DeadEvt::doit()
    {
        if (_abort)
        {
            cout << "Simulation aborted!!!" << endl;
            exit(-1);
        }
        if (_kill && _task->isActive() == true)
        {
            _task->killInstance();
        }
    }
    
} // namespace RTSim
