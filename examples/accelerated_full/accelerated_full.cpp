/*
  In this example, a simple system is simulated, consisting of two
  real-time tasks scheduled by EDF on a single processor.
*/
#include <kernel.hpp>
#include <task.hpp>
#include <fpsched.hpp>
#include <jtrace.hpp>
#include <texttrace.hpp>
#include <json_trace.hpp>
#include <ps_trace.hpp>
#include <acceleratedtask.hpp>
#include <instr.hpp>
#include <fcfsresmanager.hpp>
#include <resource.hpp>
#include <basestat.hpp>
#include <fpgakernel.hpp>

#include <iostream>
#include <time.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <boost/filesystem.hpp>

#include <randomvar.hpp>

#include "generator.h"
#include "constant.h"

#include <vector>

using namespace MetaSim;
using namespace RTSim;

const unsigned int SW_TASK_NUM = 3;

#define SIMUL_RUNS 10


const string dirRootName = "results/";

int main()
{
  try {
    // Initialize timer and folder

    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    ///////////////////

    // Create experiment folder

    string curDir = dirRootName + asctime(timeinfo);
    curDir.at(curDir.length() - 1) = '/';
    boost::filesystem::create_directories(curDir.c_str());

    //////////////////


    RandomGen randVar(time(NULL));

    overallArchitecture_t arch;

    arch.TASK_NUM = 5;
    arch.TASK_NUM_MIN = 3;
    arch.TASK_NUM_MAX = 10;
    arch.UTILIZATION_MAX = 0.8;

    arch.PERIOD_MIN = 3000;
    arch.PERIOD_MAX = 50000;
    arch.PERIOD_STEP = 1000;

    arch.A_tot = 1000;
    arch.K_RT = 10;
    arch.PARTITION_NUM = 10;
    arch.SLOT_NUM_MIN = 1;
    arch.SLOT_NUM_MAX = 10;
    arch.SLOT_NUM_STEP = 1;

    arch.SPEEDUP_MIN = 1;
    arch.SPEEDUP_MAX = 10;
    arch.SPEEDUP_STEP = 1;


    Environment * e = new Environment(&randVar);

    // SPEEDUP

    string speedupDir = curDir + "SPEEDUP/";
    boost::filesystem::create_directories(speedupDir);

    for (arch.SPEEDUP = arch.SPEEDUP_MIN;
         arch.SPEEDUP <= arch.SPEEDUP_MAX;
         arch.SPEEDUP += arch.SPEEDUP_STEP) {

      string valDir = speedupDir + to_string(arch.SPEEDUP);
      boost::filesystem::create_directories(valDir);

      writeConfigurationToFile(speedupDir, arch);

      for (unsigned int i=0; i<SIMUL_RUNS; ++i) {
        e->rebuild(arch);
        SIMUL.run(500);

        string runDir = valDir + "/" + to_string(i) + "/";
        boost::filesystem::create_directories(runDir);
        e->resultsToFile(runDir);
      }

      // dentro ci metto i risultati dei vari run


      //  generateEnvironment(strutturaParametriTaskEArchitettura);

      //for (SIM_RUNS) {
      SIMUL.run(50);
      // getResults();
      //}
    }



    delete e;

    //for (int N_task=2; i<50; i++) {

    //for (int N_SLOT=0.2; i<0.5; i++) {
    //  updateStruttura(&strutturaParametriTaskEArchitettura);

    // Crea la cartella

    //  for (SIM_RUNS) {
    //    generateEnvironment(strutturaParametriTaskEArchitettura);
    //SIMUL.run(50);
    //   getResults();
    // }
    //}

  } catch (BaseExc &e) {
    cout << e.what() << endl;
  }
}
