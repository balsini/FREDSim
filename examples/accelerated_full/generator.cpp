#include "generator.h"
#include "constant.h"

#include <limits>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <tick.hpp>

//#define WITHPSTRACE

#ifdef WITHPSTRACE
#include <ps_trace.hpp>
#endif

namespace RTSim {
  using namespace std;

  unsigned int getPeriods(unsigned int period_min,
                          unsigned int period_max,
                          unsigned int period_step,
                          RandomGen& mygen)
  {
    int Kmax = (period_max - period_min) / period_step;
    UniformVar K(0, Kmax, &mygen);
    int kvalue = K.get();
    return period_min + kvalue * period_step;
  }

  vector<double> UUnifast(int number, double MYU, RandomGen& mygen)
  {
    vector<double> result;
    double sumU = MYU;
    double UMin = 0.01;
    double NextSumU, base, exp, temp;
    UniformVar myvar(0, 1, &mygen);
    for (int i = 0; i < number - 1; i++){
      base = myvar.get();
      exp = (double)((double)1 / (double)(number - i - 1));
      temp = pow(base, exp);
      NextSumU = sumU*temp;
      if (NextSumU>sumU - UMin)
        NextSumU = sumU - UMin;
      if (NextSumU<(number - i - 1)*UMin)
        NextSumU = (number - i - 1)*UMin;

      result.push_back(sumU - NextSumU);
      sumU = NextSumU;
    }
    result.push_back(sumU);
    return result;
  }

  void writeConfigurationToFile(const string &path, const overallArchitecture_t &arch)
  {
    ofstream confFile(path + "architectureParameters.txt", std::ofstream::out);

    confFile << "TASK_MAX_K\t" << arch.TASK_MAX_K << endl;
    confFile << "PERIOD_MIN\t" << arch.PERIOD_MIN << endl;
    confFile << "PERIOD_MAX\t" << arch.PERIOD_MAX << endl;
    confFile << "A_tot\t" << arch.A_TOT << endl;
    confFile << "PARTITION_NUM\t" << arch.PARTITION_NUM << endl;
    confFile << "SLOT_NUM_MIN\t" << arch.SLOT_NUM_MIN << endl;
    confFile << "SLOT_NUM_MAX\t" << arch.SLOT_NUM_MAX << endl;
    confFile << "RHO\t" << arch.RHO << endl;
    //confFile << "SPEEDUP\t" << arch.SPEEDUP << endl;
    //confFile << "SPEEDUP_MIN\t" << arch.SPEEDUP_MIN << endl;
    //confFile << "SPEEDUP_MAX\t" << arch.SPEEDUP_MAX << endl;

    confFile.close();
  }

  Environment::Environment(RandomGen * randVar) :
    randomVar(randVar),
    pstrace(nullptr),
    softSched(nullptr),
    kern(nullptr),
    FPGA_real(nullptr),
    softSchedSW(nullptr),
    kernSW(nullptr)
  {}

  void Environment::clean()
  {
    while (responseTimeMax.size() > 0) {
      delete responseTimeMax.back();
      responseTimeMax.pop_back();
    }

    while (responseTimeMean.size() > 0) {
      delete responseTimeMean.back();
      responseTimeMean.pop_back();
    }

    while (responseTimeMaxSW.size() > 0) {
      delete responseTimeMaxSW.back();
      responseTimeMaxSW.pop_back();
    }

    while (responseTimeMeanSW.size() > 0) {
      delete responseTimeMeanSW.back();
      responseTimeMeanSW.pop_back();
    }

    partition.clear();
    partition_slot_size.clear();
    partition_slot_number.clear();
    acceleratedTaskC.clear();

    while (acceleratedTask.size() > 0) {
      delete acceleratedTask.back();
      acceleratedTask.pop_back();
    }

    while (softwareTask.size() > 0) {
      delete softwareTask.back();
      softwareTask.pop_back();
    }

    if (softSched) {
      delete softSched;
      softSched = nullptr;
    }

    if (kern) {
      delete kern;
      kern = nullptr;
    }

    if (FPGA_real) {
      delete FPGA_real;
      FPGA_real = nullptr;
    }

    if (pstrace) {
      delete pstrace;
      pstrace = nullptr;
    }

    if (softSchedSW) {
      delete softSchedSW;
      softSchedSW = nullptr;
    }

    if (kernSW) {
      delete kernSW;
      kernSW = nullptr;
    }
  }

  void environmentInit(Environment_details_t &e)
  {
    e.A_TOT = 0;
    e.FRI = FP_NONPREEMPTIVE;
    e.N_S = 0;
    e.P = 0;
    e.partition_slot_size.clear();
    e.rho = 0;
    e.slots_per_partition.clear();
    e.speedup = 0;
    e.tasks_number = 0;
    e.task_per_partition.clear();
    e.taskset_U_SW = 0;
    e.taskset_U_HW = 0;
  }


  void taskInit(task_details_t &t)
  {
    t.A = 0;
    t.C_HW = 0;
    t.C_SW_1 = 0;
    t.C_SW_2 = 0;
    t.D = 0;
    t.P = 0;
    t.T = 0;
    t.U = 0;
  }

  Environment_details_t generateEnvironment(const overallArchitecture_t &arch, RandomGen * randomVar)
  {
    Environment_details_t e;
    environmentInit(e);

    e.A_TOT = arch.A_TOT;
    e.rho = arch.RHO;
    e.speedup = arch.SPEEDUP;
    e.FRI = arch.FRI;
    e.U_HW = arch.U_HW;
    e.U_HW_UB = arch.U_HW_UB;

    //////////////////////////
    // Partitions and Slots //
    //////////////////////////

    e.P = arch.PARTITION_NUM;

    UniformVar slotsRand(arch.SLOT_NUM_MIN, arch.SLOT_NUM_MAX, randomVar);

    unsigned biggestPartitionSlots = 0;
    e.N_S = 0; // Total number of slots
    for (unsigned int i=0; i<e.P; ++i) {
      unsigned int N_S_i = slotsRand.get(); // Number of slots for partition i
      e.N_S += N_S_i;
      e.slots_per_partition.push_back(N_S_i);

      unsigned int slot_size = e.A_TOT / (e.P * N_S_i);
      e.partition_slot_size.push_back(slot_size);

      e.partition_U.push_back(e.U_HW * N_S_i);

      if (N_S_i > biggestPartitionSlots)
        biggestPartitionSlots = N_S_i;
    }

    ///////////
    // Tasks //
    ///////////

    // For each partition, the minimum number of tasks must be greater than
    // the number of slots.

    // Create an uninitialized taskset for each partition
    e.tasks_number = 0;
    for (unsigned int i=0; i<e.P; ++i) {

      UniformVar tasksRand(e.slots_per_partition.at(i) + 1,
                           e.slots_per_partition.at(i) + 1 + arch.TASK_MAX_K,
                           randomVar);

      std::vector<task_details_t> partition_taskset;

      unsigned int partition_taskset_size = tasksRand.get();

      e.tasks_number += partition_taskset_size;

      for (unsigned t = 0; t<partition_taskset_size; ++t) {
        task_details_t td;
        taskInit(td);
        partition_taskset.push_back(td);
      }

      e.task_per_partition.push_back(partition_taskset);
    }

    UniformVar tasksRandPeriod(arch.PERIOD_MIN,
                               arch.PERIOD_MAX,
                               randomVar);

    e.taskset_U_SW = arch.U_SW;
    e.taskset_U_HW = arch.U_HW;
    vector<double> utilization_factors = UUnifast(e.tasks_number, e.taskset_U_SW, *randomVar);

    vector<unsigned int> periods;
    unsigned int uf_i = 0;
    for (unsigned int p=0; p<e.task_per_partition.size(); ++p) {

      // Generates the utilization factors for each hardware task
      vector<double> utilization_factors_hw;
      bool go_on = true;
      while (go_on) {
        go_on = false;
        utilization_factors_hw = UUnifast(e.task_per_partition.at(p).size(), e.partition_U.at(p), *randomVar);
        for (unsigned int i=0; i<utilization_factors_hw.size(); ++i) {
          if (utilization_factors_hw.at(i) > e.U_HW_UB) {
            go_on = true;
            break;
          }
        }
      }

      // Assign tasks parameters
      for (unsigned int t=0; t<e.task_per_partition.at(p).size(); ++t) {
        e.task_per_partition.at(p).at(t).A = p;
        e.task_per_partition.at(p).at(t).U = utilization_factors.at(uf_i);
        e.task_per_partition.at(p).at(t).T = tasksRandPeriod.get();
        periods.push_back(e.task_per_partition.at(p).at(t).T);
        e.task_per_partition.at(p).at(t).D = e.task_per_partition.at(p).at(t).T;

        unsigned int C = e.task_per_partition.at(p).at(t).U * e.task_per_partition.at(p).at(t).T;

        UniformVar tasksCi(arch.C_SW_MIN,
                           C - arch.C_SW_MIN,
                           randomVar);
        unsigned int C1 = tasksCi.get();
        unsigned int C2 = C - C1;
        e.task_per_partition.at(p).at(t).C_SW_1 = C1;
        e.task_per_partition.at(p).at(t).C_SW_2 = C2;

        // C = U * T
        e.task_per_partition.at(p).at(t).C_HW = utilization_factors_hw.at(t) *
            e.task_per_partition.at(p).at(t).T;

        uf_i++;
      }
    }

    // Make tasks Rate Monotonic
    sort(periods.begin(), periods.end());
    for (unsigned int p=0; p<e.task_per_partition.size(); ++p) {
      for (unsigned int t=0; t<e.task_per_partition.at(p).size(); ++t) {
        auto it = std::find(periods.begin(), periods.end(), e.task_per_partition.at(p).at(t).T);
        if (it == periods.end()) {
          e.task_per_partition.at(p).at(t).P = -1;
        } else {
          e.task_per_partition.at(p).at(t).P = std::distance(periods.begin(), it);
        }
      }
    }

    return e;
  }

  void Environment::build(const Environment_details_t &ed) throw (EnvironmentExc)
  {
    clean();

#ifdef WITHPSTRACE
    static unsigned int pstraceNumber = 0;
    stringstream filename;
    filename << "PS_trace_" << std::setfill('0') << std::setw(5) << pstraceNumber++ << ".pst";
    pstrace = new PSTrace(filename.str());
#endif

    softSched = new FPScheduler;
    kern = new RTKernel(softSched);
    FPGA_real = new FPGAKernel(DISPATCHER_FIRST, ed.FRI);
    //FPGA_real = new FPGAKernel(DISPATCHER_FIRST, FP_NONPREEMPTIVE);

    softSchedSW = new FPScheduler;
    kernSW = new RTKernel(softSchedSW);


    ///////////////////////////////////
    // Creating partitions and slots //
    ///////////////////////////////////

    for (unsigned int i=0; i<ed.slots_per_partition.size(); ++i) {
      partition.push_back(FPGA_real->addPartition(ed.slots_per_partition.at(i)));
      partition_slot_number.push_back(ed.slots_per_partition.at(i));

      partition_slot_size.push_back(ed.partition_slot_size.at(i));
    }

    ////////////////////
    // Creating Tasks //
    ////////////////////

    unsigned int task_id = 0;
    for (unsigned int i=0; i<ed.task_per_partition.size(); ++i) {
      for (unsigned int j=0; j<ed.task_per_partition.at(i).size(); ++j) {


        Tick period(static_cast<int>(ed.task_per_partition.at(i).at(j).T));
        Tick deadline(static_cast<int>(ed.task_per_partition.at(i).at(j).D));

        AcceleratedTask * t = new AcceleratedTask(period,
                                                  deadline,
                                                  0,
                                                  "Task" + to_string(task_id++));
        acceleratedTask.push_back(t);


        ///////////////////////////////////////////////////////////////
        // Assigning software and hardware tasks computational times //
        ///////////////////////////////////////////////////////////////

        string at_code = "fixed(" + to_string(ed.task_per_partition.at(i).at(j).C_SW_1)
            + ");accelerate(" + to_string(ed.task_per_partition.at(i).at(j).C_HW)
            + ");fixed(" + to_string(ed.task_per_partition.at(i).at(j).C_SW_2) + ");";

        t->insertCode(at_code);

        acceleratedTaskC.push_back(pair<unsigned int, unsigned int>(ed.task_per_partition.at(i).at(j).C_SW_1 +
                                                                    ed.task_per_partition.at(i).at(j).C_SW_2,
                                                                    ed.task_per_partition.at(i).at(j).C_HW));


        ///////////////////////////////////////////////////////////////////////////
        // Assigning hardware tasks affinities (currently 1 task per partition)  //
        ///////////////////////////////////////////////////////////////////////////

        vector<Scheduler *> affinity = {partition.at(ed.task_per_partition.at(i).at(j).A)};
        t->getHW()->setAffinity(affinity);


        ////////////////////////////////
        // Set Hardware Task Priority //
        ////////////////////////////////

        t->getHW()->setFRIPriority(ed.task_per_partition.at(i).at(j).P);

        ////////////////////////////////////////////////////
        // Assigning hardware tasks reconfiguration times //
        ////////////////////////////////////////////////////

        int reconf_time = ed.partition_slot_size.at(i) / ed.rho;

        t->getHW()->setConfigurationTime(reconf_time);


        ////////////////////////
        // Linking statistics //
        ////////////////////////

        StatMax * statMax = new StatMax;
        responseTimeMax.push_back(statMax);
        t->addMaxRTStat(statMax);

        StatMean * statMean = new StatMean;
        responseTimeMean.push_back(statMean);
        t->addMeanRTStat(statMean);
#ifdef WITHPSTRACE
        pstrace->attachToTask(t);
        pstrace->attachToTask(t->getHW());
#endif
        // Creating associated software task
        PeriodicTask * st = new PeriodicTask(period,
                                             deadline,
                                             0,
                                             "SWTask" + to_string(task_id));
        softwareTask.push_back(st);

        string st_code = "fixed(" + to_string(ed.task_per_partition.at(i).at(j).C_SW_1
                                              + static_cast<unsigned int>(ed.task_per_partition.at(i).at(j).C_HW * ed.speedup)
                                              + ed.task_per_partition.at(i).at(j).C_SW_2) + ");";
#ifdef WITHPSTRACE
        pstrace->attachToTask(st);
#endif

        st->insertCode(st_code);

        StatMax * statMaxSW = new StatMax;
        responseTimeMaxSW.push_back(statMaxSW);
        st->addMaxRTStat(statMaxSW);

        StatMean * statMeanSW = new StatMean;
        responseTimeMeanSW.push_back(statMeanSW);
        st->addMeanRTStat(statMeanSW);

        /////////////////////////////
        // Adding tasks to kernels //
        /////////////////////////////

        kern->addTask(*t, to_string(ed.task_per_partition.at(i).at(j).P));
        FPGA_real->addTask(*(t->getHW()), to_string(ed.task_per_partition.at(i).at(j).P));

        kernSW->addTask(*st, to_string(ed.task_per_partition.at(i).at(j).P));

        task_id++;
      }
    }
  }


  void Environment::resultsToFile(const string &path)
  {

    ofstream statFile(path + "output.txt", std::ofstream::out);

    statFile << "#Task,\tHRT Max,\tHRT Mean\tHRT WorstN,\tSRT Max,\tSRT Mean\tSRT WorstN" << endl;

    for (unsigned int i=0; i<responseTimeMax.size(); ++i) {

      double maxHW = responseTimeMax.at(i)->getValue();
      double meanHW = responseTimeMean.at(i)->getValue();
      double maxSW = responseTimeMaxSW.at(i)->getValue();
      double meanSW = responseTimeMeanSW.at(i)->getValue();

      if (maxHW < 1)
        maxHW = std::numeric_limits<double>::max();
      if (meanHW < 1)
        meanHW = std::numeric_limits<double>::max();
      if (maxSW < 1)
        maxSW = std::numeric_limits<double>::max();
      if (meanSW < 1)
        meanSW = std::numeric_limits<double>::max();

      statFile << i << "\t"
               << maxHW << '\t'
               << meanHW << '\t'
               << maxHW / (double)acceleratedTask.at(i)->getPeriod() << '\t'
               << maxSW << '\t'
               << meanSW << '\t'
               << maxSW / (double)acceleratedTask.at(i)->getPeriod()
               << endl;
    }

    statFile.close();
  }

  void Environment::environmentToFile(const string &path)
  {

    ofstream environmentFile(path + "environment.txt", std::ofstream::out);

    environmentFile << "# Tasks" << endl;
    environmentFile << "# Name\tPeriod\tC_SW\tC_HW" << endl;

    for (unsigned int i=0; i<acceleratedTask.size(); ++i) {
      AcceleratedTask * t = acceleratedTask.at(i);

      environmentFile << t->getName() << '\t'
                      << t->getPeriod() << '\t'
                      << acceleratedTaskC.at(i).first << '\t'
                      << acceleratedTaskC.at(i).second << '\t'
                      << endl;
    }

    environmentFile << "\n# Architecture Partitions" << endl;
    environmentFile << "# ID\tSlots\tSlotSize" << endl;

    for (unsigned int i=0; i<partition.size(); ++i) {
      environmentFile << i << '\t'
                      << partition_slot_number.at(i) << '\t'
                      << partition_slot_size.at(i) << '\t'
                      << endl;
    }

    environmentFile.close();
  }


  void runStarted(const string &path, unsigned int total)
  {
    ofstream t;
    t.open(path + "progressTotal.txt", std::ofstream::out);
    t << total;
    t.close();
  }


  void runCompleted(const string &path)
  {
    static bool first_activation = true;

    ofstream d;

    if (first_activation) {
      d.open(path + "progressDone.txt", std::ofstream::out);

      first_activation = false;
    } else {
      d.open(path + "progressDone.txt", std::ofstream::out | std::ofstream::app);
    }

    d << "#" << endl;

    d.close();
  }


  long unsigned int GCD(long unsigned int a, long unsigned int b)
  {
    do {
      if(a > b) {
        a = (a-b);
      } else if (a < b) {
        b = (b-a);
      }
    } while(a!=b);
    return a;
  }

  long unsigned int lcm(long unsigned int a, long unsigned int b)
  {
    if (a == 0 || b == 0)
      return 0;

    return (a  * b) / GCD(a, b);
  }

  Tick lcm_T(const Environment_details_t &ed)
  {
    vector<long unsigned int>periods;
    for (unsigned int i=0; i<ed.task_per_partition.size(); ++i) {
      for (unsigned int j=0; j<ed.task_per_partition.at(i).size(); ++j) {
        periods.push_back(static_cast<int>(ed.task_per_partition.at(i).at(j).T));
      }
    }

    long unsigned int mcm = periods.at(0);
    for (unsigned int i=1; i<periods.size(); i++) {
      mcm = lcm(mcm, periods.at(i));
    }

    return Tick(static_cast<double>(mcm));
  }

  Tick max_T(const Environment_details_t &ed)
  {
    vector<long unsigned int>periods;
    for (unsigned int i=0; i<ed.task_per_partition.size(); ++i) {
      for (unsigned int j=0; j<ed.task_per_partition.at(i).size(); ++j) {
        periods.push_back(static_cast<int>(ed.task_per_partition.at(i).at(j).T));
      }
    }

    long unsigned int maxT = periods.at(0);
    for (unsigned int i=1; i<periods.size(); i++) {
      if (maxT < periods.at(i))
        maxT = periods.at(i);
    }

    return Tick(static_cast<double>(maxT));
  }
}
