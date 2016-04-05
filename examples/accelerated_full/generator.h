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

      double U_SW;
      double U_HW;

      unsigned int  TASK_NUM_MAX;
      unsigned int  TASK_NUM_MIN;

      unsigned int  PERIOD_MIN;
      unsigned int  PERIOD_MAX;

      double        UTILIZATION_MAX;

      ////////////////////////////
      /// FPGA                 ///
      ////////////////////////////

      unsigned int  A_TOT;          // Reconfigurable area total size

      unsigned int  PARTITION_NUM;  // Number of reconfigurable partitions

      unsigned int  SLOT_NUM_MIN;   // Minimum number of slots for each partition
      unsigned int  SLOT_NUM_MAX;   // Maximum number of slots for each partition

      double        RHO;            // Bitstream transfer Rate
      double        SPEEDUP;        // Defining C_HW and C_SW as the computational
                                    // times required to accomplish the same task
                                    // accordingly in hardware and software, the
                                    // SPEEDUP factor is identified as C_SW / C_HW
      //double        SPEEDUP_MIN;
      //double        SPEEDUP_MAX;

      unsigned int  C_SW_MIN;       // Software section's computational time
      unsigned int  C_SW_MAX;
      unsigned int  C_HW_MIN;       // Hardware section's computational time
      unsigned int  C_HW_MAX;
  };

  struct task_details_t {
      double U;             // Utilization factor
      unsigned int T;       // Period
      unsigned int D;       // Relative deadline
      unsigned int P;       // Priority
      unsigned int A;       // Affinity
      unsigned int C_SW_1;  // SW job's first computation time
      unsigned int C_SW_2;  // SW job's first computation time
      unsigned int C_HW;    // HW job's computation time
  };

  struct Environment_details_t {
      double rho;          // Bitstream transfer Rate
      double speedup;       // Defining C_HW and C_SW as the computational
                            // times required to accomplish the same task
                            // accordingly in hardware and software, the
                            // SPEEDUP factor is identified as C_SW / C_HW
      unsigned int A_TOT;   // Total area
      unsigned int P;       // Total number of partitions
      unsigned int N_S;     // Total number of slots
      std::vector<unsigned int> slots_per_partition;  // For each partition,
                            // the number of slots
      std::vector<unsigned int> partition_slot_size;  // For each partition,
                            // the size of a single slot
      bool icap_preemptive;

      double taskset_U_SW;     // SW Taskset utilization factor
      double taskset_U_HW;     // HW Taskset utilization factor
      unsigned int tasks_number;  // Total number of tasks
      std::vector<std::vector<task_details_t>> task_per_partition; // Taskset
                            // For each partition
  };

  Environment_details_t generateEnvironment(const overallArchitecture_t &arch, RandomGen * randomVar);

  void writeConfigurationToFile(const string & path, const overallArchitecture_t &arch);

  unsigned int getPeriods(unsigned int period_min, unsigned int period_max, unsigned int period_step, RandomGen& mygen);
  vector<double> UUnifast(int number, double MYU, RandomGen &mygen);

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
      vector<StatMax *> responseTime;
      vector<AcceleratedTask *> acceleratedTask;
      vector<pair<unsigned int, unsigned int>> acceleratedTaskC;

      FPScheduler * softSched;
      RTKernel * kern;
      FPGAKernel * FPGA_real;

      void clean();

    public:

      Environment(RandomGen * randVar);

      ~Environment() {
        clean();
      }

      void build(const Environment_details_t &ed) throw (EnvironmentExc);
      void build_old(const overallArchitecture_t &arch) throw (EnvironmentExc);
      void resultsToFile(const string &path);
      void environmentToFile(const string &path);
  };


}
