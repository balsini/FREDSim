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
#include <simul.hpp>
#include <kernel.hpp>
#include <task.hpp>
#include <schedinstr.hpp>

namespace RTSim {

    SchedInstr::SchedInstr(Task * f, const string& s, const string &n)
        : Instr(f, n), _endEvt(this), _threEvt(f, this) 
    {}

    Instr* SchedInstr::createInstance(vector<string> &par)
    {
        return new SchedInstr(dynamic_cast<Task *>(Entity::_find(par[1])),par[0]);
    }

    void SchedInstr::endRun() 
    {
        _endEvt.drop(); 
        _threEvt.drop();
    }

    void SchedInstr::schedule()
    {
        DBGENTER(_INSTR_DBG_LEV);
        DBGPRINT("Scheduling SchedInstr named: " << getName());

        _endEvt.post(SIMUL.getTime());    
    }

    void SchedInstr::deschedule()
    {
        DBGTAG(_INSTR_DBG_LEV,"SchedInstr::deschedule()");
        _endEvt.drop();
    }

    void SchedInstr::setTrace(Trace *t) 
    {
        _endEvt.addTrace(t); 
        _threEvt.addTrace(t);
    }

    void SchedInstr::onEnd() 
    {
        DBGENTER(_INSTR_DBG_LEV);

        DBGPRINT("Ending SchedInstr named: " << getName());

        _father->onInstrEnd();

        RTKernel *k = dynamic_cast<RTKernel *>(_father->getKernel());

        if (k == NULL) throw BaseExc("Kernel not found!");

        DBGPRINT("Before lowing threshold for task " << _father->getName());

        k->disableThreshold();
        k->dispatch();

        DBGPRINT("After lowing threshold for task " << _father->getName());

        _threEvt.process();

    

    }

}
