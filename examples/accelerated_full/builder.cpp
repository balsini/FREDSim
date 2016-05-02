#include "builder.h"

#include <limits>
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

  void Environment::build(const Environment_details_t &ed) throw (EnvironmentExc)
  {
    clean();

    /*
    static unsigned int pstraceNumber = 0;
    stringstream filename;
    filename << "PS_trace_" << std::setfill('0') << std::setw(5) << pstraceNumber++ << ".pst";
    pstrace = new PSTrace(filename.str());
    */

    softSched = new FPScheduler;
    kern = new RTKernel(softSched);
    FPGA_real = new FPGAKernel(DISPATCHER_FIRST, static_cast<FRIAlgorithm>(ed.FRI));
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

        //pstrace->attachToTask(t);
        //pstrace->attachToTask(t->getHW());

        // Creating associated software task
        PeriodicTask * st = new PeriodicTask(period,
                                             deadline,
                                             0,
                                             "SWTask" + to_string(task_id));
        softwareTask.push_back(st);

        string st_code = "fixed(" + to_string(ed.task_per_partition.at(i).at(j).C_SW_1
                                              + static_cast<unsigned int>(ed.task_per_partition.at(i).at(j).C_HW * ed.speedup)
                                              + ed.task_per_partition.at(i).at(j).C_SW_2) + ");";

        //pstrace->attachToTask(st);

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


  void Environment::addAnalysisResults(bool schedulable)
  {
    analysis_schedulable = schedulable;
  }


  void Environment::resultsToFile(const string &path)
  {

    ofstream statFile(path + "output.txt", std::ofstream::out);

    statFile << "#Task,\tHRT Max,\tHRT Mean\tHRT WorstN,\tSRT Max,\tSRT Mean\tSRT WorstN\tAnResult" << endl;

    for (unsigned int i=0; i<responseTimeMax.size(); ++i) {

      double maxHW = responseTimeMax.at(i)->getValue();
      double meanHW = responseTimeMean.at(i)->getValue();
      double maxSW = responseTimeMaxSW.at(i)->getValue();
      double meanSW = responseTimeMeanSW.at(i)->getValue();
      unsigned int sched = analysis_schedulable ? 1 : 0;

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
               << maxSW / (double)acceleratedTask.at(i)->getPeriod() << '\t'
               << sched
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
