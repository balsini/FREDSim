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
  /**
       Models a simple periodic task. It's a simpler interface to
       Task.
    */
  class HardwareTask: public Task
  {
      AcceleratedTask * _parent;

      AccelerateInstr * ai;

    public:
      HardwareTask(Tick rdl, Tick ph = 0,
                   const std::string &name = "",
                   long qs = 1000, Tick maxC=0);
      void setParent(AcceleratedTask * p);
      void onEndInstance(MetaSim::Event *);
      static HardwareTask* createInstance(vector<string>& par);

      void setAccelerateInstr(AccelerateInstr * i);
  };

} // namespace RTSim

#endif
