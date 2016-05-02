#ifndef __GENERATOR_H__
#define __GENERATOR_H__

#include <vector>
#include <exception>
#include <string>
#include <algorithm>

#include <sstream>

//---------------------------------------------------------------------------
// Bugfix for MinGW:
// http://stackoverflow.com/questions/12975341/to-string-is-not-a-member-of-std-says-so-g
namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

//---------------------------------------------------------------------------

class architectureException: public std::exception
{
    std::string _msg;
  public:
    architectureException(const std::string &msg) : _msg("Error (architectureException): " + msg) {}
    virtual const char* what() const throw()
    {
      return _msg.c_str();
    }
};

class UniformVarRand {
  protected:
    double _min;
    double _max;
  public:
    UniformVarRand(double min, double max) : _min(min), _max(max) {
      if (min > max)
        throw architectureException("UniformVarRand: min > max");
    }
    virtual double get();
};

template <class T>
class BucketVar {
    std::vector<T>_bucket;
  public:
    BucketVar(const std::vector<T>bucket)
    {
      set(bucket);
    }
    BucketVar() {}
    void set(const std::vector<T>bucket)
    {
      _bucket = bucket;
      std::random_shuffle(_bucket.begin(), _bucket.end());
    }
    T get(T min, T max);
};

enum FRIAlgorithmBuilder {
  FP_PREEMPTIVE_B = 0,
  FP_NONPREEMPTIVE_B = 1,
  TB_PREEMPTIVE_B = 2,
  TB_NONPREEMPTIVE_B = 3
};

struct overallArchitecture_t {
    //string name;
    unsigned int processes;
    unsigned int runs;

    unsigned int variables;

    ////////////////////////////
    /// Task                 ///
    ////////////////////////////

    double U_SW;  // The utilization factor of SW taskset
    std::vector<double> U_SW_list;

    double U_HW;  // The utilization factor that can be handled by every
    // single slot
    std::vector<double> U_HW_list;

    double U_HW_UB; // The utilization factor upperbound for each slot

    FRIAlgorithmBuilder  FRI;
    std::vector<FRIAlgorithmBuilder> FRI_list;

    double        TASK_MAX_K;
    unsigned int  TASK_APPENDED;
    std::vector<unsigned int>TASK_APPENDED_list;

    unsigned int  PERIOD_MIN;
    unsigned int  PERIOD_MAX;
    std::vector<unsigned int>PERIOD_break_list;  // For N partitions, there exist
    std::vector<unsigned int>PERIOD_bucket;

    // N-1 period breaks.
    // For each partition i, the periods of the tasks are chosen in the range:
    // [break[i-1], break[i]], but:
    //   if i==0: break[i-1] = PERIOD_MIN.
    //   if i==(N-1): break[i] = PERIOD_MAX.

    ////////////////////////////
    /// FPGA                 ///
    ////////////////////////////

    unsigned int  A_TOT;          // Reconfigurable area total size
    std::vector<unsigned int>A_TOT_list;

    unsigned int  PARTITION_NUM;  // Number of reconfigurable partitions

    unsigned int  SLOT_NUM_MIN;   // Minimum number of slots for each partition
    unsigned int  SLOT_NUM_MAX;   // Maximum number of slots for each partition

    double        RHO;            // Bitstream transfer Rate
    double        SPEEDUP;        // Defining C_HW and C_SW as the computational
    // times required to accomplish the same task
    // accordingly in hardware and software, the
    // SPEEDUP factor is identified as C_SW / C_HW
    std::vector<double>SPEEDUP_list;

    unsigned int  C_SW_MIN;       // Software section's computational time
    //unsigned int  C_HW_MIN;       // Hardware section's computational time
    //unsigned int  C_HW_MAX;
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

struct extraTasks_details_t {
    unsigned int N;       // Number of new tasks
    double U_SW;          // Software utilization factor
    double U_HW;          // Hardware utilization factor
};

struct Environment_details_t {
    double rho;          // Bitstream transfer Rate
    double speedup;       // Defining C_HW and C_SW as the computational
    // times required to accomplish the same task
    // accordingly in hardware and software, the
    // SPEEDUP factor is identified as C_SW / C_HW
    double U_HW;          // The utilization factor handled by a single slot
    double U_HW_UB;       // The utilization factor upperbound for each slot
    unsigned int A_TOT;   // Total area
    unsigned int P;       // Total number of partitions
    unsigned int N_S;     // Total number of slots
    std::vector<unsigned int> slots_per_partition;  // For each partition,
    // the number of slots
    std::vector<unsigned int> partition_slot_size;  // For each partition,
    // the size of a single slot
    std::vector<double> partition_U;  // For each partition,
    // the total utilization factor handled
    FRIAlgorithmBuilder FRI;

    BucketVar<unsigned int>bucket;

    double taskset_U_SW;     // SW Taskset utilization factor
    double taskset_U_HW;     // HW Taskset utilization factor
    unsigned int tasks_number;  // Total number of tasks
    std::vector< std::vector<task_details_t> > task_per_partition; // Taskset
    // For each partition
    std::vector< std::vector<task_details_t> > SW_taskset; // Classical software taskset
};

std::vector<double> UUnifast(int number, double MYU, double UB = 1.0);
Environment_details_t generateEnvironment(const overallArchitecture_t &arch);
Environment_details_t environmentAddTask(const Environment_details_t &env,
                                         const overallArchitecture_t &arch,
                                         const extraTasks_details_t &t);
void printEnvironment(const Environment_details_t &e);

#endif
