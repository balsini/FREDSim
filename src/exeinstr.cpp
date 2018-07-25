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
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

#include <factory.hpp>
#include <simul.hpp>
#include <strtoken.hpp>

#include <cpu.hpp>
#include <exeinstr.hpp>
#include <task.hpp>

namespace RTSim {

    using namespace MetaSim;
    using namespace std;
    using namespace parse_util;

    ExecInstr::ExecInstr(Task *f,
                         unique_ptr<RandomVar> c,
                         const string &wl,
                         const string &n) :
        Instr(f, n), flag(false),
        cost(std::move(c)),
        workload(wl),
        execdTime(0),
        currentCost(0),
        actTime(0),
        lastTime(0),
        executing(false),
        _endEvt(this) 
    {
        DBGTAG(_INSTR_DBG_LEV,"ExecInstr constructor");
    }

    
    ExecInstr::ExecInstr(const ExecInstr &other) : 
        Instr(other), flag(false),
        cost(other.cost->clone()),
        execdTime(0),
        currentCost(0),
        actTime(0),
        lastTime(0),
        executing(false),
        _endEvt(this)
    {
        DBGTAG(_INSTR_DBG_LEV, "ExecInstr copy constructor");
    }

    
    ExecInstr::~ExecInstr()
    {
        DBGTAG(_INSTR_DBG_LEV, "ExecInstr::~ExecInstr() called");
    }    
    

    Instr *ExecInstr::createInstance(const vector<string> &par)
    {
        Instr *temp = 0;

        Task *task = dynamic_cast<Task *>(Entity::_find(par[par.size() - 1]));
        //if (isdigit((par[0].c_str())[0])) {
        if (isdigit(par[0][0])) {
            temp = new FixedInstr(task, atoi(par[0].c_str()));
        }
        else {
            string token = get_token(par[0]);
            string p = get_param(par[0]);
            vector<string> parms = split_param(p);

            unique_ptr<RandomVar> var(FACT(RandomVar).create(token,parms));
    
            if (var.get() == 0) throw ParseExc("ExecInstr", par[0]);

            temp = new ExecInstr(task, std::move(var));
        }
        return temp;
    }

    void ExecInstr::newRun() 
    {
        actTime = lastTime = 0;
        flag = true;
        execdTime = 0;
        executing = false;
    }

    void ExecInstr::endRun() 
    {
        _endEvt.drop();
    }

    Tick ExecInstr::getExecTime() const 
    { 
        Tick t = SIMUL.getTime();
        if (executing) return (execdTime + t - lastTime);
        else return execdTime;
    }

    Tick ExecInstr::getDuration() const 
    { 
        return (Tick)cost->get();
    }

    Tick ExecInstr::getWCET() const throw(RandomVar::MaxException)
    { 
        return (Tick) cost->getMaximum();
    }

    void ExecInstr::schedule() throw (InstrExc)
    {
        DBGENTER(_INSTR_DBG_LEV);

        Tick t = SIMUL.getTime();
        lastTime = t;
        executing = true;

        if (flag) {
  
            DBGPRINT_3("Initializing ExecInstr ",
                       getName(), 
                       " at first schedule.");
            DBGPRINT_2("Time executed during the prev. instance: ", 
                       execdTime);

            execdTime = 0; 
            actTime = 0;
            flag = false;
            currentCost = Tick(cost->get());

            DBGPRINT_2("Time to execute for this instance: ",
                       currentCost);
        }

        CPU *p = _father->getCPU();
        if (!dynamic_cast<CPU *>(p)) 
            throw InstrExc("No CPU!", "ExeInstr::schedule()");

        double currentSpeed = p->getSpeed();
  
        Tick tmp = 0;
        if (((double)currentCost) > actTime)
            tmp = (Tick) ceil( ((double)currentCost - actTime)/currentSpeed);
        
        _endEvt.post(t + tmp);
	      
        DBGPRINT("End of ExecInstr::schedule() ");
        
    }

    void ExecInstr::deschedule()
    {
        Tick t = SIMUL.getTime();

        DBGENTER(_INSTR_DBG_LEV);
        DBGPRINT("Descheduling ExecInstr named: " << getName());

        _endEvt.drop();

        if (executing) {
            CPU *p = _father->getOldCPU();
            if (!dynamic_cast<CPU *>(p)) 
                throw InstrExc("No CPU!", 
                               "ExeInstr::deschedule()");
    
            double currentSpeed = p->getSpeed();

            actTime += ((double)(t - lastTime))*currentSpeed;// number of cycles
            execdTime += (t - lastTime);// number of ticks
            lastTime = t; 
        }
        executing = false;

        
    }

    // void ExecInstr::setTrace(Trace *t) {
    //     attach_stat(*t, _endEvt); 
    //     //_endEvt.addTrace(t);
    // }

    void ExecInstr::onEnd() 
    {
        DBGENTER(_INSTR_DBG_LEV);
        DBGPRINT("Ending ExecInstr named: " << getName());

        Tick t = SIMUL.getTime();
        execdTime += t - lastTime;
        flag = true;
        executing = false;
        lastTime = t;
        actTime = 0;
        _endEvt.drop();

        DBGPRINT("internal data set... now calling the _father->onInstrEnd()");

        _father->onInstrEnd();        
    }


    void ExecInstr::reset() 
    {
        DBGENTER(_INSTR_DBG_LEV);

        actTime = lastTime = 0;
        flag = true;
        execdTime = 0;
        _endEvt.drop();

        DBGPRINT("internal data reset...");
    }

    void ExecInstr::refreshExec(double oldSpeed, double newSpeed){
        Tick t = SIMUL.getTime();
        _endEvt.drop();
        actTime += ((double)(t - lastTime))*oldSpeed;
        execdTime += (t - lastTime);
        lastTime = t;
   
        Tick tmp = 0;
        if (((double)currentCost) > actTime)
            tmp = (Tick) ceil ((((double) currentCost) - actTime)/newSpeed);
	   
        _endEvt.post(t + tmp);
    }

    /*---------------------------- */

    FixedInstr::FixedInstr(Task *t,
                           Tick duration,
                           const string &wl,
                           const std::string &n) :
        ExecInstr(t,
                  unique_ptr<DeltaVar>(new DeltaVar(duration)),
                  wl,
                  n)
    {}

    unique_ptr<Instr> FixedInstr::createInstance(const vector<string> &par)
    {
        Task *task = dynamic_cast<Task *>(Entity::_find(par[par.size() - 1]));
        string workload = par.size() <= 2 ? "" : par[1];
        return unique_ptr<FixedInstr>(new FixedInstr(task, stoi(par[0]), workload));
    }
}
