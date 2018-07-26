/*
  In this example, a simple system is simulated, to provide
  an evaluation of different workloads running on an big.LITTLE
  Odroid-XU3 embedded board.
*/

#include <cstring>
#include <string>

#include <mrtkernel.hpp>
#include <cpu.hpp>
#include <edfsched.hpp>
#include <jtrace.hpp>
#include <texttrace.hpp>
#include <json_trace.hpp>
#include <tracepower.hpp>
#include <rttask.hpp>
#include <instr.hpp>
#include <powermodel.hpp>

using namespace MetaSim;
using namespace RTSim;

/* ./energy [OPP little] [OPP big] [workload] */

int main(int argc, char *argv[])
{
    unsigned int OPP_little = 0;
    unsigned int OPP_big = 0;
    string workload = "idle";

    if (argc == 4) {
        OPP_little = stoi(argv[1]);
        OPP_big = stoi(argv[2]);
        workload = argv[3];
    }

    cout << "OPPs: [" << OPP_little << ", " << OPP_big << "]" << endl;
    cout << "Workload: [" << workload << "]" << endl;

    try {
        SIMUL.dbg.enable("All");
        SIMUL.dbg.setStream("debug.txt");

        TextTrace ttrace("trace.txt");
        JSONTrace jtrace("trace.json");

        vector<TracePowerConsumption *> ptrace;
        vector<EDFScheduler *> schedulers;
        vector<RTKernel *> kernels;

        /* ------------------------- Creating CPUs -------------------------*/
        for (unsigned int i=0; i<4; ++i) {
            /* Create LITTLE CPUs */
            vector<double> V = {
                0.92, 0.919643, 0.919357, 0.918924, 0.95625, 0.9925, 1.02993, 1.0475, 1.08445, 1.12125, 1.15779, 1.2075, 1.25625
            };
            vector<unsigned int> F = {
                200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400
            };
            string cpu_name = "LITTLE_" + to_string(i);

            cout << "Creating CPU: " << cpu_name << endl;

            PowerModel *pm = new PowerModelBP(V[V.size() - 1], F[F.size() - 1]);
            {
                PowerModelBP::PowerModelBPParams idle_pp = {0.000100131, 0.0765454, 81.1412, 1.51697e-10};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("idle", idle_pp);
                PowerModelBP::PowerModelBPParams bzip2_pp = {0.000116787, 154.351, 0.000108753, 6.277e-10};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("bzip2", bzip2_pp);
                PowerModelBP::PowerModelBPParams hash_pp = {0.00122777, 401.483, 0.00135651, 2.40986e-10};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("hash", hash_pp);
                PowerModelBP::PowerModelBPParams encrypt_pp = {1.0406e-23, 82.9392, 0.554462, 8.33564e-10};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("encrypt", encrypt_pp);
                PowerModelBP::PowerModelBPParams decrypt_pp = {6.34944e-6, 175.93, 0.289347, 4.83442e-10};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("decrypt", decrypt_pp);
                PowerModelBP::PowerModelBPParams cachekiller_pp = {0.00800217, 244.519, 0.00769861, 3.11737e-10};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("cachekiller", cachekiller_pp);
            }

            CPU *c = new CPU(cpu_name, V, F, pm);
            if (OPP_little >= V.size())
                throw(-1);
            c->setOPP(OPP_little);
            c->setWorkload("idle");
            pm->setCPU(c);
            TracePowerConsumption *power_trace = new TracePowerConsumption(c, 1, "power_" + cpu_name + ".txt");
            ptrace.push_back(power_trace);

            EDFScheduler *edfsched = new EDFScheduler;
            schedulers.push_back(edfsched);

            RTKernel *kern = new RTKernel(edfsched, "", c);
            kernels.push_back(kern);
        }

        for (unsigned int i=0; i<4; ++i) {
            /* Create big CPUs */
            vector<double> V = {
                0.916319, 0.915475, 0.915102, 0.91498, 0.91502, 0.90375, 0.916562, 0.942543, 0.96877, 0.994941, 1.02094, 1.04648, 1.05995, 1.08583, 1.12384, 1.16325, 1.20235, 1.2538, 1.33287
            };
            vector<unsigned int> F = {
                200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000
            };

            string cpu_name = "BIG_" + to_string(i);

            cout << "Creating CPU: " << cpu_name << endl;

            PowerModel *pm = new PowerModelBP(V[V.size() - 1], F[F.size() - 1]);
            {
                PowerModelBP::PowerModelBPParams idle_pp = {0.0152928, 0.00178247, 31.5652, 1.8031e-9};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("idle", idle_pp);
                PowerModelBP::PowerModelBPParams bzip2_pp = {0.0240903, 112.597, 0.701912, 2.32641e-9};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("bzip2", bzip2_pp);
                PowerModelBP::PowerModelBPParams hash_pp = {0.0487379, 10.0036, 3.39211, 9.60823e-9};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("hash", hash_pp);
                PowerModelBP::PowerModelBPParams encrypt_pp = {0.0233073, 28.5073, 0.94835, 8.64423e-9};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("encrypt", encrypt_pp);
                PowerModelBP::PowerModelBPParams decrypt_pp = {0.0267308, 52.6299, 0.647324, 5.74505e-9};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("decrypt", decrypt_pp);
                PowerModelBP::PowerModelBPParams cachekiller_pp = {0.068222, 17.9726, 1.40253, 6.4189e-9};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("cachekiller", cachekiller_pp);
            }

            CPU *c = new CPU(cpu_name, V, F, pm);
            if (OPP_big >= V.size())
                throw(-1);
            c->setOPP(OPP_big);
            c->setWorkload("idle");
            pm->setCPU(c);
            TracePowerConsumption *power_trace = new TracePowerConsumption(c, 1, "power_" + cpu_name + ".txt");
            ptrace.push_back(power_trace);

            EDFScheduler *edfsched = new EDFScheduler;
            schedulers.push_back(edfsched);

            RTKernel *kern = new RTKernel(edfsched, "", c);
            kernels.push_back(kern);
        }


        /* ------------------------- Creating tasks -------------------------*/

        PeriodicTask *t;

        /* LITTLE */

        t = new PeriodicTask(500, 100, 0, "Task_LITTLE_0");
        t->insertCode("fixed(500," + workload + ");");
        kernels[0]->addTask(*t, "");
        ttrace.attachToTask(*t);
        jtrace.attachToTask(*t);

        /* big */

        t = new PeriodicTask(500, 100, 0, "Task_big_0");
        t->insertCode("fixed(500," + workload + ");");
        kernels[4]->addTask(*t, "");
        ttrace.attachToTask(*t);
        jtrace.attachToTask(*t);

        cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        cout << "Running simulation!" << endl;

        SIMUL.run(50000);
    } catch (BaseExc &e) {
        cout << e.what() << endl;
    }
}
