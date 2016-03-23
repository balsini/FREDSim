#include <cctype>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

#include <factory.hpp>
#include <simul.hpp>
#include <strtoken.hpp>

#include <cpu.hpp>
#include <accelerate_instr.hpp>
#include <acceleratedtask.hpp>
#include <hardwaretask.hpp>

namespace RTSim {

  using namespace MetaSim;
  using namespace std;
  using namespace parse_util;

  AccelerateInstr::AccelerateInstr(Task *f, Tick d) :
    Instr(f),
    computation(d),
    suspEvt(this, &AccelerateInstr::onSuspend),
    resumeEvt(this, &AccelerateInstr::onEnd)
  {
    DBGTAG(_INSTR_DBG_LEV,"AccelerateInstr");
  }

  Instr *AccelerateInstr::createInstance(vector<string> &par)
  {
    if (par.size() != 2) throw parse_util::ParseExc("AccelerateInstr::createInstance", "Wrong number of arguments");

    Task *task = dynamic_cast<Task *>(Entity::_find(par[1]));
    return new AccelerateInstr(task, atoi(par[0].c_str()));
  }

  void AccelerateInstr::endRun()
  {
    suspEvt.drop();
    resumeEvt.drop();
  }

  void AccelerateInstr::onSuspend(Event *evt)
  {
    AcceleratedTask * accelerated = dynamic_cast<AcceleratedTask *>(_father);
    HardwareTask * hw = accelerated->getHW();
    // Update hardware instructions

    hw->discardInstrs();
    string code =
        "spinLock(ICAP);configure("
        + to_string(static_cast<long>(hw->getReconfigurationTime()))
        + ");spinUnlock(ICAP);"
        + "fixed("
        + to_string(static_cast<long>(computation))
        + ");";
    hw->insertCode(code);

    // Activate hardware task

    hw->setAccelerateInstr(this);
    hw->arrEvt.post(SIMUL.getTime());

    // Suspend accelerated task

    AbsKernel *k = _father->getKernel();
    k->suspend(_father);
    k->dispatch();
  }

  void AccelerateInstr::onEnd(Event *evt)
  {
    _father->onInstrEnd();

    AbsKernel *k = _father->getKernel();

    k->onArrival(_father);
  }

  void AccelerateInstr::schedule()
  {
    suspEvt.process();
  }

}
