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

  //class BlockEvt;
  //class UnblockEvt;

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
      bool configure;

      unsigned int FRI_priority;

    public:

      BlockEvt blockEvt;
      UnblockEvt unblockEvt;
      ConfigEvt configEvt;
      EndConfigEvt endConfigEvt;

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

      void setCPU(CPU * cpu) {
        _cpu = cpu;
        endEvt.setPriority(cpu->getIndex());
      }
      CPU *getCPU() const;
      CPU *getOldCPU() const {
        return getCPU();
      }

      void schedule();
      void deschedule();

      void addMaxRTStat(StatMax * s) { maxRT = s; }
      void addMeanRTStat(StatMean * s) { meanRT = s; }
      void addMinRTStat(StatMin * s) { minRT = s; }
      void startConfiguration() {
        configEvt.process();
      }
      void setFRIPriority(unsigned int p) { FRI_priority = p; }
      unsigned int getFRIPriority() { return FRI_priority; }
      void endConfiguration() {
        endConfigEvt.process();
      }
      void setBlocked() {
        blockEvt.process();
      }
      void setUnblocked() {
        unblockEvt.process();
      }

      void configRequired(bool c) {
        configure = c;
      }
      bool configRequired() {
        return configure;
      }

      void endRun()
      {
        while (!arrQueue.empty()) {
          arrQueue.pop_front();
        }
        arrEvt.drop();
        endEvt.drop();
        schedEvt.drop();
        deschedEvt.drop();
        fakeArrEvt.drop();
        deadEvt.drop();

        blockEvt.drop();
        unblockEvt.drop();
        configEvt.drop();
        endConfigEvt.drop();
      }
  };

} // namespace RTSim

#endif
