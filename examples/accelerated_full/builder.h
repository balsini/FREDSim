#ifndef __BUILDER_H__
#define __BUILDER_H__

#include <vector>

#include <tick.hpp>
#include <randomvar.hpp>

#include <basestat.hpp>
#include <acceleratedtask.hpp>
#include <fpsched.hpp>
#include <fpgakernel.hpp>
#include <kernel.hpp>
#include <ps_trace.hpp>

#include "builder.h"
#include "generator.h"

namespace RTSim {
  using namespace MetaSim;

  void runCompleted(const string &path);
  void runStarted(const string &path, unsigned int total);
  Tick lcm_T(const Environment_details_t &ed);
  Tick max_T(const Environment_details_t &ed);

  void writeConfigurationToFile(const string & path, const overallArchitecture_t &arch);

  unsigned int getPeriods(unsigned int period_min, unsigned int period_max, unsigned int period_step, RandomGen& mygen);

  class EnvironmentExc : public BaseExc {
    public:
      EnvironmentExc(string msg) :BaseExc(msg, "Environment", "generator.cpp") {}
  };

  class Environment {

      RandomGen * randomVar;

      PSTrace * pstrace;

      vector<Scheduler *> partition;
      vector<unsigned int> partition_slot_size;
      vector<unsigned int> partition_slot_number;
      vector<StatMax *> responseTimeMax;
      vector<StatMean *> responseTimeMean;
      vector<StatMax *> responseTimeMaxSW;
      vector<StatMean *> responseTimeMeanSW;
      vector<AcceleratedTask *> acceleratedTask;
      vector<PeriodicTask *> softwareTask;
      vector<pair<unsigned int, unsigned int>> acceleratedTaskC;

      FPScheduler * softSched;
      RTKernel * kern;
      FPGAKernel * FPGA_real;

      FPScheduler * softSchedSW;
      RTKernel * kernSW;

      void clean();

    public:

      Environment(RandomGen * randVar);

      ~Environment() {
        clean();
      }

      void build(const Environment_details_t &ed) throw (EnvironmentExc);
      void resultsToFile(const string &path);
      void environmentToFile(const string &path);
  };
}

#endif
