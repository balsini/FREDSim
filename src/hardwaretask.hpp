#ifndef __HARDWARETASK_HPP__
#define __HARDWARETASK_HPP__

#include <cstdlib>
#include <cstring>

#include <regvar.hpp>
#include <simul.hpp>

#include <task.hpp>

namespace RTSim {

  using namespace std;
  using namespace MetaSim;

  class AcceleratedTask;
  class AccelerateInstr;
  class FPGAKernel;
  /**
       Models a simple periodic task. It's a simpler interface to
       Task.
    */
  class HardwareTask: public Task
  {
      AcceleratedTask * _parent;
      FPGAKernel * _fpgakernel;

      AccelerateInstr * _ai;

      unsigned int _affinity;

    public:
      HardwareTask(Tick rdl, Tick ph = 0,
                   const std::string &name = "",
                   long qs = 1000, Tick maxC=0);
      void setParent(AcceleratedTask * p);
      void onEndInstance(MetaSim::Event *);
      void handleArrival(Tick arr);
      static HardwareTask* createInstance(vector<string>& par);

      void setFPGAKernel(FPGAKernel * f) { _fpgakernel = f; }
      unsigned int getAffinity() const { return _affinity; }
      void setAffinity(unsigned int affinity) { _affinity = affinity; }

      void setAccelerateInstr(AccelerateInstr * i);
  };

} // namespace RTSim

#endif
