#include "generator.h"
#include "constant.h"

#include <fstream>

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
    double UMin = 0.005;
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

    confFile << "TASK_NUM_MAX\t" << arch.TASK_NUM_MAX << endl;
    confFile << "PERIOD_MIN\t" << arch.PERIOD_MIN << endl;
    confFile << "PERIOD_MAX\t" << arch.PERIOD_MAX << endl;
    confFile << "UTILIZATION_MAX\t" << arch.UTILIZATION_MAX << endl;
    confFile << "A_tot\t" << arch.A_tot << endl;
    confFile << "PARTITION_NUM\t" << arch.PARTITION_NUM << endl;
    confFile << "SLOT_NUM_MIN\t" << arch.SLOT_NUM_MIN << endl;
    confFile << "SLOT_NUM_MAX\t" << arch.SLOT_NUM_MAX << endl;
    confFile << "K_RT\t" << arch.K_RT << endl;
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

  void Environment::rebuild(const overallArchitecture_t &arch) throw (EnvironmentExc)
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

    pstrace = new PSTrace("PS_trace_" + to_string(pstraceNumber++) + ".pst");

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

      unsigned int slot_size = local_arch.A_tot / (local_arch.PARTITION_NUM * N_S_i);
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

    unsigned int TASK_NUM_MIN = N_S;

    UniformVar tasksRand(TASK_NUM_MIN, local_arch.TASK_NUM_MAX, randomVar);

    unsigned int TASK_NUM = tasksRand.get();

    for (unsigned int i=0; i<TASK_NUM; ++i) {

      UniformVar periodRand(local_arch.PERIOD_MIN, local_arch.PERIOD_MAX, randomVar);

      Tick taskPeriod(periodRand.get());

      AcceleratedTask * t = new AcceleratedTask(taskPeriod,
                                                taskPeriod,
                                                0,
                                                "Task" + to_string(i));


      UniformVar C_SW_Rand(local_arch.C_SW_MIN, local_arch.C_SW_MAX, randomVar);
      UniformVar C_HW_Rand(local_arch.C_HW_MIN, local_arch.C_HW_MAX, randomVar);

      unsigned int softwareComputation = C_SW_Rand.get();
      unsigned int hardwareComputation = C_HW_Rand.get();

      t->insertCode("fixed(" + to_string(softwareComputation / 2)
                    + ");accelerate(" + to_string(hardwareComputation)
                    + ");fixed(" + to_string(softwareComputation / 2) + ");");


      UniformVar tasksToPartitionRand(0, N_S, randomVar);

      unsigned int partition_index;
      unsigned int taskToPartitionResult = tasksToPartitionRand.get();
      unsigned int counter = 0;
      for (partition_index=0; partition_index<partition.size(); ++partition_index) {
        counter += partition_slot_number.at(partition_index);
        if (taskToPartitionResult < counter)
          break;
      }

      Tick configTime(local_arch.K_RT * partition_slot_size.at(partition_index));

      t->getHW()->setConfigurationTime(configTime);

      vector<Scheduler *> affinity = {partition.at(partition_index)};
      t->getHW()->setAffinity(affinity);


      StatMax * stat = new StatMax;
      responseTime.push_back(stat);
      t->addMaxRTStat(stat);


      pstrace->attachToTask(t);
      pstrace->attachToTask(t->getHW());


      kern->addTask(*t, to_string(i));
      FPGA_real->addTask(*(t->getHW()), to_string(i));

      acceleratedTask.push_back(t);
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
    environmentFile << "# Name\tPeriod\t" << endl;

    for (unsigned int i=0; i<acceleratedTask.size(); ++i) {
      AcceleratedTask * t = acceleratedTask.at(i);

      environmentFile << t->getName() << '\t'
                      << t->getPeriod() << '\t'
                      //<< t->getComputationTime() << '\t'
                      << endl;
    }

    environmentFile << "\n# Architecture" << endl;
    environmentFile << "# Partition" << endl;

    for (unsigned int i=0; i<partition.size(); ++i) {
      environmentFile << i << '\t'
                      << endl;
    }

    environmentFile.close();
  }

}
