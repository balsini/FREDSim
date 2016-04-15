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

#define SIMUL_RUNS  200
#define DURATION    (500 * 1000)

#define THREAD_NUMBER   (48 * 2)


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


        arch.TASK_NUM_MAX = 2;

        arch.PERIOD_MIN = 200;
        arch.PERIOD_MAX = 500;

        arch.A_TOT = 1000;
        arch.RHO = 10;

        arch.PARTITION_NUM = 2;
        arch.SLOT_NUM_MIN = 2;
        arch.SLOT_NUM_MAX = 2;

        arch.SPEEDUP = 1;

        arch.C_SW_MIN = 1;

        Environment * e = new Environment(&randVar);

        // TASK_NUM

        vector<FRIAlgorithm> fa = {FP_PREEMPTIVE,
                                   FP_NONPREEMPTIVE,
                                   TB_PREEMPTIVE,
                                   TB_NONPREEMPTIVE};

        pid_t child, changed_child;
        vector<pid_t> children;
        int status;

        for (unsigned int i=0; i<fa.size(); ++i) {

            arch.FRI = fa.at(i);

            string speedupDir = curDir + "U_HW_" + to_string(arch.FRI) + "/";
            boost::filesystem::create_directories(speedupDir);

            arch.U_SW = 0.1;
            arch.U_HW = 0.3;
            arch.U_HW_UB = 1;

            for (arch.U_HW = 0.05;
                 arch.U_HW <= 0.95;
                 arch.U_HW += 0.05) {

                string valDir = speedupDir + to_string(arch.U_HW);
                boost::filesystem::create_directories(valDir);

                writeConfigurationToFile(speedupDir, arch);


                for (unsigned int i=0; i<SIMUL_RUNS; ++i) {

                        string runDir = valDir + "/" + to_string(i) + "/";
                        boost::filesystem::create_directories(runDir);

                        Environment_details_t ed = generateEnvironment(arch, &randVar);
                        e->build(ed);
                        e->environmentToFile(runDir);

                    if ((child = fork()) == 0) {
                        // Child process
                        SIMUL.run(DURATION);

                        e->resultsToFile(runDir);

                        delete e;
                        return 0;

                    } else {
                        children.push_back(child);

                        while (children.size() >= THREAD_NUMBER)
                        {
                            changed_child = wait(&status);
                            if (changed_child > 0)
                            {
                                children.erase(find(children.begin(),
                                                    children.end(),
                                                    changed_child));
                            } else {
                                cout << "wait returned " << changed_child << endl;
                            }
                        }
                    }
                }
            }
        }

        while (children.size() > 0)
        {
            changed_child = wait(&status);
            if (changed_child > 0)
            {
                children.erase(find(children.begin(),
                                    children.end(),
                                    changed_child));
            } else {
                cout << "wait returned " << changed_child << endl;
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
