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
    suspEvt(this, &AccelerateInstr::onSuspend),
    resumeEvt(this, &AccelerateInstr::onEnd),
    computation(d)
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
    // TODO
    // use SuspendInstr as example

    AbsKernel *k = _father->getKernel();

    AcceleratedTask * t = dynamic_cast<AcceleratedTask *>(_father);

    // Update hardware instructions

    t->getHW()->discardInstrs();
    string code = "fixed(" + to_string((long)computation) + ");";
    t->getHW()->insertCode(code);

    // Activate hardware task

    AbsKernel *fpga = t->getHW()->getKernel();

    dynamic_cast<HardwareTask *>(t->getHW())->setAccelerateInstr(this);
    t->getHW()->activate();

    k->suspend(_father);
    k->dispatch();
  }

  void AccelerateInstr::onEnd(Event *evt)
  {
    // TODO
    // use SuspendInstr as example

    _father->onInstrEnd();

    AbsKernel *k = _father->getKernel();

    k->onArrival(_father);
  }

  void AccelerateInstr::schedule()
  {
    suspEvt.process();
  }

}
