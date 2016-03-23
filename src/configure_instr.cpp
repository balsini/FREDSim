#include <configure_instr.hpp>

#include <simul.hpp>

#include <exeinstr.hpp>
#include <task.hpp>
#include <hardwaretask.hpp>

namespace RTSim {

  using namespace MetaSim;
  using namespace std;
  using namespace parse_util;

  Instr *ConfigureInstr::createInstance(vector<string> &par)
  {
    Task *task = dynamic_cast<Task *>(Entity::_find(par[1]));
    return new ConfigureInstr(task, atoi(par[0].c_str()));
  }

  void ConfigureInstr::schedule() throw (InstrExc)
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


    dynamic_cast<HardwareTask *>(_father)->startConfiguration();
  }

  void ConfigureInstr::onEnd()
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

    dynamic_cast<HardwareTask *>(_father)->endConfiguration();
  }

}
