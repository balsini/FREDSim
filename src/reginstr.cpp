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
#include <strtoken.hpp>

#include <exeinstr.hpp>
#include <threinstr.hpp>
#include <schedinstr.hpp>
#include <waitinstr.hpp>
#include <suspend_instr.hpp>
#include <accelerate_instr.hpp>

namespace RTSim {

    const Instr::BASE_KEY_TYPE ExecName("delay");
    const Instr::BASE_KEY_TYPE FixedName("fixed");

    const Instr::BASE_KEY_TYPE WaitName1("wait");
    const Instr::BASE_KEY_TYPE WaitName2("lock");

    const Instr::BASE_KEY_TYPE SignalName1("signal");
    const Instr::BASE_KEY_TYPE SignalName2("unlock");

    const Instr::BASE_KEY_TYPE ThreName("threshold");
    const Instr::BASE_KEY_TYPE ScheduleName("schedule");

    const Instr::BASE_KEY_TYPE SuspendName("suspend");

    const Instr::BASE_KEY_TYPE AccelerateName("accelerate");

    /** 
        This namespace should never be used by the user. Contains
        functions to initialize the abstract factory that builds
        instructions @see Task::insertCode()
    */ 
    namespace __instr_stub
    {
        static registerInFactory<Instr, ExecInstr, Instr::BASE_KEY_TYPE>
        registerMe1(ExecName);
                
        static registerInFactory<Instr, FixedInstr, Instr::BASE_KEY_TYPE>
        registerMe2(FixedName);

        static registerInFactory<Instr, WaitInstr, Instr::BASE_KEY_TYPE>
        registerWait1(WaitName1);
                
        static registerInFactory<Instr, WaitInstr, Instr::BASE_KEY_TYPE>
        registerWait2(WaitName2);
                
        static registerInFactory<Instr, SignalInstr, Instr::BASE_KEY_TYPE>
        registerSignal1(SignalName1);
                
        static registerInFactory<Instr, SignalInstr, Instr::BASE_KEY_TYPE>
        registerSignal2(SignalName2);

        static registerInFactory<Instr, ThreInstr, Instr::BASE_KEY_TYPE>
        registerThre(ThreName);
                
        static registerInFactory<Instr, SchedInstr, Instr::BASE_KEY_TYPE>
        registerSchedule(ScheduleName);

        static registerInFactory<Instr, SuspendInstr, Instr::BASE_KEY_TYPE>
        registerSuspend(SuspendName);

        static registerInFactory<Instr, AccelerateInstr, Instr::BASE_KEY_TYPE>
        registerAccelerate(AccelerateName);
    }

    void __reginstr_init() {}

} // namespace RTSim

