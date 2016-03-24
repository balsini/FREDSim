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

#include <fpgakernel.hpp>
#include <task.hpp>
#include <spinlockinstr.hpp>

namespace RTSim {

    SpinLockInstr::SpinLockInstr(Task * f, const char *r, int nr, const string &n)
        : Instr(f, n), _res(r),  _endEvt(this), _numberOfRes(nr)
    {}

    SpinLockInstr::SpinLockInstr(Task * f, const string &r, int nr, const string &n)
        : Instr(f, n), _res(r),  _endEvt(this), _numberOfRes(nr)
    {}

    Instr* SpinLockInstr::createInstance(vector<string> &par)
    {
        return new SpinLockInstr(dynamic_cast<Task *>(Entity::_find(par[1])), par[0]);
    }

    void SpinLockInstr::endRun()
    {
      _endEvt.drop();
    }

    void SpinLockInstr::schedule()
    {
        DBGENTER(_INSTR_DBG_LEV);
        DBGPRINT("Scheduling SpinLockInstr named: " << getName());

        _endEvt.post(SIMUL.getTime());
    }

    void SpinLockInstr::deschedule()
    {}

    void SpinLockInstr::setTrace(Trace *t)
    {
      _endEvt.addTrace(t);
    }

    void SpinLockInstr::onEnd()
    {
        DBGENTER(_INSTR_DBG_LEV);

        FPGAKernel *k = dynamic_cast<FPGAKernel *>(_father->getKernel());

        if (k == nullptr)
          throw BaseExc("Kernel not found!");

        if (k->requestResource(_father, _res, _numberOfRes))
          _father->onInstrEnd();
    }

    SpinUnlockInstr::SpinUnlockInstr(Task *f,  const char *r, int nr, const string &n)
        : Instr(f, n), _res(r), _endEvt(this), _numberOfRes(nr)
    {}

    SpinUnlockInstr::SpinUnlockInstr(Task *f, const string &r, int nr, const string &n)
        : Instr(f, n), _res(r), _endEvt(this), _numberOfRes(nr)
    {}

    Instr* SpinUnlockInstr::createInstance(vector<string> &par)
    {
        return new SpinUnlockInstr(dynamic_cast<Task *>(Entity::_find(par[1])), par[0]);
    }

    void SpinUnlockInstr::endRun()
    {
      _endEvt.drop();
    }

    void SpinUnlockInstr::schedule()
    {
      _endEvt.post(SIMUL.getTime());
    }

    void SpinUnlockInstr::deschedule()
    {
    }

    void SpinUnlockInstr::setTrace(Trace *t)
    {
      _endEvt.addTrace(t);
    }

    void SpinUnlockInstr::onEnd()
    {
        DBGENTER(_INSTR_DBG_LEV);

        _father->onInstrEnd();

        FPGAKernel *k = dynamic_cast<FPGAKernel *>(_father->getKernel());

        if (k == nullptr) {
            throw BaseExc("SpinUnlockInstr has no kernel set!");
        }
        
        k->releaseResource(_father, _res, _numberOfRes);
    }

}
