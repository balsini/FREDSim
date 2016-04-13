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
#include <unistd.h>
#include <sys/wait.h>

#include <randomvar.hpp>

#include "generator.h"
#include "constant.h"

#include <vector>

using namespace MetaSim;
using namespace RTSim;

#define SIMUL_RUNS  500
#define DURATION    (500 * 1000)

#define THREAD_NUMBER   12


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
        replace(curDir.begin(), curDir.end(), ' ', '_');
        boost::filesystem::create_directories(curDir.c_str());

        //////////////////


        RandomGen randVar(time(NULL));

        overallArchitecture_t arch;


        arch.TASK_NUM_MAX = 8;

        arch.PERIOD_MIN = 200;
        arch.PERIOD_MAX = 500;

        arch.A_TOT = 1000;
        arch.RHO = 10;

        arch.PARTITION_NUM = 4;
        arch.SLOT_NUM_MIN = 2;
        arch.SLOT_NUM_MAX = 5;

        arch.SPEEDUP = 10;

        arch.C_SW_MIN = 2;

        arch.U_SW = 0.5;
        arch.U_HW = 0.5;
        arch.U_HW_UB = 0.5;


        Environment * e = new Environment(&randVar);

        // TASK_NUM

        vector<FRIAlgorithm> fa = {FP_PREEMPTIVE,
                                   FP_NONPREEMPTIVE,
                                   TB_PREEMPTIVE,
                                   TB_NONPREEMPTIVE};

        pid_t last_thread;
        vector<pid_t> spawned_threads;

        for (unsigned int i=0; i<fa.size(); ++i) {

            arch.FRI = fa.at(i);

            string speedupDir = curDir + "U_HW_" + to_string(arch.FRI) + "/";
            boost::filesystem::create_directories(speedupDir);


            for (arch.U_HW = 0.05;
                 arch.U_HW <= 0.95;
                 arch.U_HW += 0.05) {

                string valDir = speedupDir + to_string(arch.U_HW);
                boost::filesystem::create_directories(valDir);

                writeConfigurationToFile(speedupDir, arch);

                if ((last_thread = fork()) == 0) {
                    // Child process
                    for (unsigned int i=0; i<SIMUL_RUNS; ++i) {
                        string runDir = valDir + "/" + to_string(i) + "/";
                        boost::filesystem::create_directories(runDir);

                        Environment_details_t ed = generateEnvironment(arch, &randVar);
                        e->build(ed);
                        e->environmentToFile(runDir);

                        SIMUL.run(DURATION);

                        e->resultsToFile(runDir);
                    }
                    goto thread_finished;
                } else {
                    spawned_threads.push_back(last_thread);
                    pid_t changed_process;
                    if (spawned_threads.size() >= THREAD_NUMBER) {
                        int status;
                        changed_process = wait(&status);
                        spawned_threads.erase(find(spawned_threads.begin(), spawned_threads.end(), changed_process));
                    }
                }
            }
        }

        int status;
        while (spawned_threads.size() > 0) {
            wait(&status);
            spawned_threads.pop_back();
        }

        thread_finished:

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
