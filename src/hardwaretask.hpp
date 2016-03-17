#ifndef __HARDWARETASK_HPP__
#define __HARDWARETASK_HPP__

#include <cstdlib>
#include <cstring>

#include <regvar.hpp>
#include <simul.hpp>

#include <task.hpp>

#include <stdint.h>

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
      AcceleratedTask *_parent;

      AccelerateInstr *_ai;

      vector<Scheduler *> _affinity;
      Tick _configurationTime;

      Tick lastArrival;

      CPU *_cpu;

      StatMax *maxRT;
      StatMean *meanRT;
      StatMin *minRT;

    public:
      HardwareTask(Tick rdl, Tick ph = 0,
                   const std::string &name = "",
                   long qs = 1000, Tick maxC=0);
      void setParent(AcceleratedTask * p);
      void onEndInstance(MetaSim::Event *);
      void handleArrival(Tick arr);
      static HardwareTask* createInstance(vector<string>& par);

      vector<Scheduler *> getAffinity() { return _affinity; }
      void setAffinity(vector<Scheduler *> affinity) { _affinity = affinity; }

      void setConfigurationTime(Tick ct) { _configurationTime = ct; }
      Tick getReconfigurationTime() const { return _configurationTime; }

      void setAccelerateInstr(AccelerateInstr * i);

      void setCPU(CPU * cpu) { _cpu = cpu; }
      CPU *getCPU() const;

      void addMaxRTStat(StatMax * s) { maxRT = s; }
      void addMeanRTStat(StatMean * s) { meanRT = s; }
      void addMinRTStat(StatMin * s) { minRT = s; }
  };

} // namespace RTSim

#endif
