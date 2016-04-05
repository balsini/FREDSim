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

#define SIMUL_RUNS 3


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

    arch.TASK_NUM_MIN = 5;
    arch.TASK_NUM_MAX = 5;
    arch.UTILIZATION_MAX = 0.8;

    arch.PERIOD_MIN = 200;
    arch.PERIOD_MAX = 500;

    arch.A_TOT = 200;
    arch.RHO = 10;

    arch.PARTITION_NUM = 2;
    arch.SLOT_NUM_MIN = 2;
    arch.SLOT_NUM_MAX = 2;

    arch.SPEEDUP = 10;
    //arch.SPEEDUP_MIN = 1;
    //arch.SPEEDUP_MAX = 3;

    arch.C_SW_MIN = 10;
    arch.C_SW_MAX = 50;
    arch.C_HW_MIN = 10;
    arch.C_HW_MAX = 50;

    arch.U_SW = 0.4;
    arch.U_HW = 0.6;


    Environment * e = new Environment(&randVar);

    // TASK_NUM

    string speedupDir = curDir + "TASK_NUM/";
    boost::filesystem::create_directories(speedupDir);

    for (arch.TASK_NUM_MIN = 5;
         arch.TASK_NUM_MIN <= 10;
         arch.TASK_NUM_MIN += 1) {

      arch.TASK_NUM_MAX = arch.TASK_NUM_MIN;

      string valDir = speedupDir + to_string(arch.TASK_NUM_MIN);
      boost::filesystem::create_directories(valDir);

      writeConfigurationToFile(speedupDir, arch);

      for (unsigned int i=0; i<SIMUL_RUNS; ++i) {
        string runDir = valDir + "/" + to_string(i) + "/";
        boost::filesystem::create_directories(runDir);

        Environment_details_t ed = generateEnvironment(arch, &randVar);
        e->build(ed);
        e->environmentToFile(runDir);

        SIMUL.run(5000);

        e->resultsToFile(runDir);
      }
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
