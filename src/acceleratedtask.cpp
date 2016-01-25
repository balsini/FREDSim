#include "acceleratedtask.hpp"

namespace RTSim {
  AcceleratedTask::AcceleratedTask(Tick iat, Tick rdl, Tick ph,
                                   const std::string &name, long qs)
    : Task(new DeltaVar(iat), rdl, ph, name, qs), period(iat)
  {
    ht = std::unique_ptr<HardwareTask>(new HardwareTask(0, 0, "Hardware"+name));
    ht.get()->insertCode("fixed(1);");
  }

  AcceleratedTask* AcceleratedTask::createInstance(vector<string>& par)
  {
    Tick i = Tick(par[0]);
    Tick d = Tick(par[1]);
    Tick p = Tick(par[2]);
    const char* n = "";
    if (par.size() > 2) n = par[3].c_str();
    long q = 100;
    if (par.size() > 4) q = atoi(par[4].c_str());

    return new AcceleratedTask(i, d, p, n, q);
  }
}
