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

    confFile << "TASK_NUM\t" << arch.TASK_NUM << endl;
    confFile << "TASK_NUM_MIN\t" << arch.TASK_NUM_MIN << endl;
    confFile << "TASK_NUM_MAX\t" << arch.TASK_NUM_MAX << endl;
    confFile << "PERIOD_MIN\t" << arch.PERIOD_MIN << endl;
    confFile << "PERIOD_MAX\t" << arch.PERIOD_MAX << endl;
    confFile << "PERIOD_STEP\t" << arch.PERIOD_STEP << endl;
    confFile << "UTILIZATION_MAX\t" << arch.UTILIZATION_MAX << endl;
    confFile << "A_tot\t" << arch.A_tot << endl;
    confFile << "PARTITION_NUM\t" << arch.PARTITION_NUM << endl;
    confFile << "SLOT_NUM_MIN\t" << arch.SLOT_NUM_MIN << endl;
    confFile << "SLOT_NUM_MAX\t" << arch.SLOT_NUM_MAX << endl;
    confFile << "SLOT_NUM_STEP\t" << arch.SLOT_NUM_STEP << endl;
    confFile << "K_RT\t" << arch.K_RT << endl;
    confFile << "SPEEDUP\t" << arch.SPEEDUP << endl;
    confFile << "SPEEDUP_MIN\t" << arch.SPEEDUP_MIN << endl;
    confFile << "SPEEDUP_MAX\t" << arch.SPEEDUP_MAX << endl;
    confFile << "SPEEDUP_STEP\t" << arch.SPEEDUP_STEP << endl;
    confFile << "SLOT_NUM_STEP\t" << arch.SLOT_NUM_STEP << endl;
    confFile << "SLOT_NUM_STEP\t" << arch.SLOT_NUM_STEP << endl;

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

    while (partition.size() > 0) {
      // Partitions are owned by the FPGAKernel,
      // so are destructed by the owner
      partition.pop_back();
    }

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

  void Environment::rebuild(const overallArchitecture_t &arch)
  {
    clean();

    pstrace = new PSTrace("X.pst");
    UniformVar slotsRand(arch.SLOT_NUM_MIN, arch.SLOT_NUM_MAX, randomVar);

    softSched = new FPScheduler;
    kern = new RTKernel(softSched);
    FPGA_real = new FPGAKernel(DISPATCHER_FIRST);

    for (unsigned int i=0; i<arch.PARTITION_NUM; ++i) {
      partition.push_back(FPGA_real->addPartition(slotsRand.get()));
    }

    for (unsigned int i=0; i<arch.TASK_NUM; ++i) {
      AcceleratedTask * t = new AcceleratedTask(10, 10, 0, "Task" + to_string(i)); // TODO usare le funzioni scritte sopra per i periodi


      t->insertCode("fixed(1);accelerate(4);fixed(1);");
      t->getHW()->setConfigurationTime(2);


      vector<Scheduler *> affinity = {partition.at(0)}; // TODO
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

    for (unsigned int i=0; i<responseTime.size(); ++i) {
      statFile << i << "\t" << responseTime.at(i)->getValue() << endl;
    }

    statFile.close();
  }

}
