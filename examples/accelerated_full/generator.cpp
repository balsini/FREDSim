#include "generator.h"
#include "constant.h"

#include <sstream>
#include <fstream>
#include <iomanip>
#include <tick.hpp>

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

    confFile << "TASK_NUM_MIN\t" << arch.TASK_NUM_MIN << endl;
    confFile << "TASK_NUM_MAX\t" << arch.TASK_NUM_MAX << endl;
    confFile << "PERIOD_MIN\t" << arch.PERIOD_MIN << endl;
    confFile << "PERIOD_MAX\t" << arch.PERIOD_MAX << endl;
    confFile << "UTILIZATION_MAX\t" << arch.UTILIZATION_MAX << endl;
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
    FPGA_real(nullptr)
  {}

  void Environment::clean()
  {
    while (responseTime.size() > 0) {
      delete responseTime.back();
      responseTime.pop_back();
    }

    partition.clear();
    partition_slot_size.clear();
    partition_slot_number.clear();
    acceleratedTaskC.clear();

    while (acceleratedTask.size() > 0) {
      delete acceleratedTask.back();
      acceleratedTask.pop_back();
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
  }

  void environmentInit(Environment_details_t &e)
  {
    e.A_TOT = 0;
    e.icap_preemptive = false;
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

    //////////////////////////
    // Partitions and Slots //
    //////////////////////////

    e.P = arch.PARTITION_NUM;

    UniformVar slotsRand(arch.SLOT_NUM_MIN, arch.SLOT_NUM_MAX, randomVar);

    e.N_S = 0; // Total number of slots
    for (unsigned int i=0; i<arch.PARTITION_NUM; ++i) {
      unsigned int N_S_i = slotsRand.get(); // Number of slots for partition i
      e.N_S += N_S_i;
      e.slots_per_partition.push_back(N_S_i);

      unsigned int slot_size = e.A_TOT / (e.P * N_S_i);
      e.partition_slot_size.push_back(slot_size);
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
                           arch.TASK_NUM_MAX,
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
    vector<double> utilization_factors_hw = UUnifast(e.tasks_number, e.taskset_U_HW, *randomVar);

    vector<unsigned int> periods;
    unsigned int uf_i = 0;
    for (unsigned int p=0; p<e.task_per_partition.size(); ++p) {
      for (unsigned int t=0; t<e.task_per_partition.at(p).size(); ++t) {
        e.task_per_partition.at(p).at(t).A = p;
        e.task_per_partition.at(p).at(t).U = utilization_factors.at(uf_i);
        e.task_per_partition.at(p).at(t).T = tasksRandPeriod.get();
        periods.push_back(e.task_per_partition.at(p).at(t).T);
        e.task_per_partition.at(p).at(t).D = e.task_per_partition.at(p).at(t).T;

        unsigned int C = e.task_per_partition.at(p).at(t).U * e.task_per_partition.at(p).at(t).T;

        UniformVar tasksCi(1,
                           C-1,
                           randomVar);
        unsigned int C1 = tasksCi.get();
        unsigned int C2 = C - C1;
        e.task_per_partition.at(p).at(t).C_SW_1 = C1;
        e.task_per_partition.at(p).at(t).C_SW_2 = C2;

        // U T = C
        e.task_per_partition.at(p).at(t).C_HW = utilization_factors_hw.at(uf_i) *
            e.task_per_partition.at(p).at(t).T;

        uf_i++;
      }
    }

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

  void Environment::build_old(const overallArchitecture_t &arch) throw (EnvironmentExc)
  {
    overallArchitecture_t local_arch = arch;

    static unsigned int pstraceNumber = 0;

    if (local_arch.PERIOD_MIN > local_arch.PERIOD_MAX) {
      throw EnvironmentExc("Minimum period should be smaller or equal to the maximum");
    }
    if (local_arch.SLOT_NUM_MIN > local_arch.SLOT_NUM_MAX) {
      throw EnvironmentExc("Minimum number of slots should be smaller or equal to the maximum");
    }
    //if (local_arch.SPEEDUP_MIN > local_arch.SPEEDUP_MAX) {
    //  throw EnvironmentExc("Minimum speedup should be smaller or equal to the maximum");
    //}

    clean();


    stringstream filename;
    filename << "PS_trace_" << std::setfill('0') << std::setw(5) << pstraceNumber++ << ".pst";
    pstrace = new PSTrace(filename.str());

    softSched = new FPScheduler;
    kern = new RTKernel(softSched);
    FPGA_real = new FPGAKernel(DISPATCHER_FIRST);


    ///////////////////////////////////
    // Creating partitions and slots //
    ///////////////////////////////////

    UniformVar slotsRand(local_arch.SLOT_NUM_MIN, local_arch.SLOT_NUM_MAX, randomVar);

    unsigned int N_S = 0; // Total number of slots

    for (unsigned int i=0; i<local_arch.PARTITION_NUM; ++i) {
      unsigned int N_S_i = slotsRand.get(); // Number of slots for partition i
      N_S += N_S_i;
      partition.push_back(FPGA_real->addPartition(N_S_i));
      partition_slot_number.push_back(N_S_i);

      unsigned int slot_size = local_arch.A_TOT / (local_arch.PARTITION_NUM * N_S_i);
      partition_slot_size.push_back(slot_size);
    }


    ////////////////////
    // Creating Tasks //
    ////////////////////

    if (local_arch.TASK_NUM_MAX < N_S) {
      throw EnvironmentExc("Maximum number of task ("
                           + to_string(local_arch.TASK_NUM_MAX)
                           + ") cannot be smaller than the total number of slots ("
                           + to_string(N_S) + ")");
    }

    UniformVar tasksRand(local_arch.TASK_NUM_MIN, local_arch.TASK_NUM_MAX, randomVar);

    unsigned int TASK_NUM = tasksRand.get();

    for (unsigned int i=0; i<TASK_NUM; ++i) {

      UniformVar periodRand(local_arch.PERIOD_MIN, local_arch.PERIOD_MAX, randomVar);

      Tick taskPeriod(periodRand.get());

      AcceleratedTask * t = new AcceleratedTask(taskPeriod,
                                                taskPeriod,
                                                0,
                                                "Task" + to_string(i));
      acceleratedTask.push_back(t);


      ///////////////////////////////////////////////////////////////
      // Assigning software and hardware tasks computational times //
      ///////////////////////////////////////////////////////////////

      UniformVar C_SW_Rand(local_arch.C_SW_MIN, local_arch.C_SW_MAX, randomVar);
      UniformVar C_HW_Rand(local_arch.C_HW_MIN, local_arch.C_HW_MAX, randomVar);

      unsigned int softwareComputationPartial = C_SW_Rand.get() / 2;
      unsigned int hardwareComputation = C_HW_Rand.get();

      t->insertCode("fixed(" + to_string(softwareComputationPartial)
                    + ");accelerate(" + to_string(hardwareComputation)
                    + ");fixed(" + to_string(softwareComputationPartial) + ");");

      acceleratedTaskC.push_back(pair<unsigned int, unsigned int>(softwareComputationPartial * 2, hardwareComputation));


      ///////////////////////////////////////////////////////////////////////////
      // Assigning hardware tasks affinities (currently 1 task per partition)  //
      ///////////////////////////////////////////////////////////////////////////

      UniformVar tasksToPartitionRand(0, N_S, randomVar);

      unsigned int partition_index;
      unsigned int taskToPartitionResult = tasksToPartitionRand.get();
      unsigned int counter = 0;
      for (partition_index=0; partition_index<partition.size(); ++partition_index) {
        counter += partition_slot_number.at(partition_index);
        if (taskToPartitionResult < counter)
          break;
      }

      vector<Scheduler *> affinity = {partition.at(partition_index)};
      t->getHW()->setAffinity(affinity);


      ////////////////////////////////////////////////////
      // Assigning hardware tasks reconfiguration times //
      ////////////////////////////////////////////////////

      Tick configTime(partition_slot_size.at(partition_index) / local_arch.RHO);
      t->getHW()->setConfigurationTime(configTime);


      ////////////////////////
      // Linking statistics //
      ////////////////////////

      StatMax * stat = new StatMax;
      responseTime.push_back(stat);
      t->addMaxRTStat(stat);


      pstrace->attachToTask(t);
      pstrace->attachToTask(t->getHW());


      /////////////////////////////
      // Adding tasks to kernels //
      /////////////////////////////

      kern->addTask(*t, to_string(i));
      FPGA_real->addTask(*(t->getHW()), to_string(i));
    }
  }


  void Environment::build(const Environment_details_t &ed) throw (EnvironmentExc)
  {
    static unsigned int pstraceNumber = 0;

    clean();


    stringstream filename;
    filename << "PS_trace_" << std::setfill('0') << std::setw(5) << pstraceNumber++ << ".pst";
    pstrace = new PSTrace(filename.str());

    softSched = new FPScheduler;
    kern = new RTKernel(softSched);
    FPGA_real = new FPGAKernel(DISPATCHER_FIRST);


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

    /*
    if (local_arch.TASK_NUM_MAX < N_S) {
      throw EnvironmentExc("Maximum number of task ("
                           + to_string(local_arch.TASK_NUM_MAX)
                           + ") cannot be smaller than the total number of slots ("
                           + to_string(N_S) + ")");
    }
    */


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

        t->insertCode("fixed(" + to_string(ed.task_per_partition.at(i).at(j).C_SW_1)
                      + ");accelerate(" + to_string(ed.task_per_partition.at(i).at(j).C_HW)
                      + ");fixed(" + to_string(ed.task_per_partition.at(i).at(j).C_SW_2) + ");");

        acceleratedTaskC.push_back(pair<unsigned int, unsigned int>(ed.task_per_partition.at(i).at(j).C_SW_1 +
                                                                    ed.task_per_partition.at(i).at(j).C_SW_2,
                                                                    ed.task_per_partition.at(i).at(j).C_HW));


        ///////////////////////////////////////////////////////////////////////////
        // Assigning hardware tasks affinities (currently 1 task per partition)  //
        ///////////////////////////////////////////////////////////////////////////

        vector<Scheduler *> affinity = {partition.at(ed.task_per_partition.at(i).at(j).A)};
        t->getHW()->setAffinity(affinity);


        ////////////////////////////////////////////////////
        // Assigning hardware tasks reconfiguration times //
        ////////////////////////////////////////////////////

        int reconf_time = ed.partition_slot_size.at(i) / ed.rho;

        t->getHW()->setConfigurationTime(reconf_time);


        ////////////////////////
        // Linking statistics //
        ////////////////////////

        StatMax * stat = new StatMax;
        responseTime.push_back(stat);
        t->addMaxRTStat(stat);


        pstrace->attachToTask(t);
        pstrace->attachToTask(t->getHW());


        /////////////////////////////
        // Adding tasks to kernels //
        /////////////////////////////

        kern->addTask(*t, to_string(i));
        FPGA_real->addTask(*(t->getHW()), to_string(i));

      }
    }
  }


  void Environment::resultsToFile(const string &path)
  {

    ofstream statFile(path + "response_times.txt", std::ofstream::out);

    statFile << "#Task ID,\tResponse Time" << endl;

    for (unsigned int i=0; i<responseTime.size(); ++i) {
      statFile << i << "\t" << responseTime.at(i)->getValue() << endl;
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

}
