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
#ifndef __PSTRACE_HPP__
#define __PSTRACE_HPP__

#include <fstream>
#include <iostream>
#include <string>

#include <baseexc.hpp>
#include <basetype.hpp>
#include <event.hpp>
#include <particle.hpp>
#include <trace.hpp>

#include <rttask.hpp>
#include <hardwaretask.hpp>
#include <taskevt.hpp>

namespace RTSim {
    class PSTrace {
    protected:
        std::ofstream fd;
        bool first_event;

        void writeTaskEvent(Task &tt, const std::string &evt_name);

    public:
        PSTrace(const std::string& name);
        ~PSTrace();

        void probe(ArrEvt& e);
        void probe(EndEvt& e);
        void probe(SchedEvt& e);
        void probe(DeschedEvt& e);
        void probe(DeadEvt& e);
        void probe(BlockEvt& e);
        void probe(UnblockEvt& e);
        void probe(ConfigEvt& e);
        void probe(EndConfigEvt& e);

        void attachToTask(Task* t);
    };
}

#endif
