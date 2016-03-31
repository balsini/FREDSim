#include <vector>

#include <tick.hpp>
#include <randomvar.hpp>

#include <basestat.hpp>
#include <acceleratedtask.hpp>
#include <fpsched.hpp>
#include <fpgakernel.hpp>
#include <kernel.hpp>
#include <ps_trace.hpp>

namespace RTSim {
  using namespace MetaSim;

  struct overallArchitecture_t {
      ////////////////////////////
      /// Task                 ///
      ////////////////////////////

      unsigned int  TASK_NUM;
      unsigned int  TASK_NUM_MIN;
      unsigned int  TASK_NUM_MAX;

      unsigned int  PERIOD_MIN;
      unsigned int  PERIOD_MAX;
      unsigned int  PERIOD_STEP;

      double        UTILIZATION_MAX;

      ////////////////////////////
      /// FPGA                 ///
      ////////////////////////////

      unsigned int  A_tot;          // Reconfigurable area total size

      unsigned int  PARTITION_NUM;  // Number of reconfigurable partitions

      unsigned int  SLOT_NUM_MIN;   // Minimum number of slots for each partition
      unsigned int  SLOT_NUM_MAX;   // Maximum number of slots for each partition
      unsigned int  SLOT_NUM_STEP;

      double        K_RT;           // Bitstream transfer Rate
      double        SPEEDUP;        // Defining C_HW and C_SW as the computational
      // times required to accomplish the same task
      // accordingly in hardware and software, the
      // SPEEDUP factor is identified as C_SW / C_HW
      double        SPEEDUP_MIN;
      double        SPEEDUP_MAX;
      double        SPEEDUP_STEP;
  };

  void writeConfigurationToFile(const string & path, const overallArchitecture_t &arch);

  unsigned int getPeriods(unsigned int period_min, unsigned int period_max, unsigned int period_step, RandomGen& mygen);
  vector<double> UUnifast(int number, double MYU, RandomGen &mygen);

  class Environment {

      RandomGen * randomVar;

      PSTrace * pstrace;

      vector<Scheduler *> partition;
      vector<StatMax *> responseTime;
      vector<AcceleratedTask *> acceleratedTask;

      FPScheduler * softSched;
      RTKernel * kern;
      FPGAKernel * FPGA_real;

      void clean();

    public:

      Environment(RandomGen * randVar);

      ~Environment() {
        clean();
      }

      void rebuild(const overallArchitecture_t &arch);
      void resultsToFile(const string &path);
  };
}
