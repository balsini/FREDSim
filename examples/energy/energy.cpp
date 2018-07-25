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

int main(int argc, char *argv[])
{
    try {
        SIMUL.dbg.enable("All");
        SIMUL.dbg.setStream("debug.txt");

        TextTrace ttrace("trace.txt");
        JSONTrace jtrace("trace.json");

        customCPUFactory *cpuFactory = new customCPUFactory();
        vector<TracePowerConsumption *> ptrace;

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
                PowerModelBP::PowerModelBPParams idle_pp = {0.000383117, 0.00884979, 64.8351, 8.15464e-10};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("idle", idle_pp);
                PowerModelBP::PowerModelBPParams bzip2_pp = {8.15795e-6, 296.934, 114.496, 3.35267e-10};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("bzip2", bzip2_pp);
                PowerModelBP::PowerModelBPParams hash_pp = {5.26822e-6, 236.14, 78.4937, 4.4112e-10};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("hash", hash_pp);
                PowerModelBP::PowerModelBPParams encrypt_pp = {3.10408e-6, 82.1798, 92.1112, 8.51862e-10};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("encrypt", encrypt_pp);
                PowerModelBP::PowerModelBPParams decrypt_pp = {0.00115974, 169.45, 143.07, 4.76291e-10};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("decrypt", decrypt_pp);
                PowerModelBP::PowerModelBPParams cachekiller_pp = {0.00453355, 217.205, 66.896, 4.27884e-10};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("cachekiller", cachekiller_pp);
            }

            CPU *c = new CPU(cpu_name, V, F, pm);
            c->setOPP(F.size() - 1);
            c->setWorkload("idle");
            pm->setCPU(c);
            cpuFactory->addCPU(c);
            TracePowerConsumption *power_trace = new TracePowerConsumption(c, 10, "power_" + cpu_name + ".txt");
            ptrace.push_back(power_trace);
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
                PowerModelBP::PowerModelBPParams idle_pp = {0.0615501, 0.181869, 70.899, 3.25146e-9};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("idle", idle_pp);
                PowerModelBP::PowerModelBPParams bzip2_pp = {0.0721914, 101.4, 142.631, 2.547e-9};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("bzip2", bzip2_pp);
                PowerModelBP::PowerModelBPParams hash_pp = {0.0985725, 62.2083, 282.31, 1.85528e-9};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("hash", hash_pp);
                PowerModelBP::PowerModelBPParams encrypt_pp = {0.0654654, 258.958, 385.494, 1.04853e-9};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("encrypt", encrypt_pp);
                PowerModelBP::PowerModelBPParams decrypt_pp = {0.0539495, 98.6102, 73.9266, 4.03708e-9};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("decrypt", decrypt_pp);
                PowerModelBP::PowerModelBPParams cachekiller_pp = {0.132446, 26.0672, 98.1562, 3.66153e-9};
                dynamic_cast<PowerModelBP *>(pm)->setWorkloadParams("cachekiller", cachekiller_pp);
            }

            CPU *c = new CPU(cpu_name, V, F, pm);
            c->setOPP(F.size() - 1);
            c->setWorkload("idle");
            pm->setCPU(c);
            cpuFactory->addCPU(c);
            TracePowerConsumption *power_trace = new TracePowerConsumption(c, 10, "power_" + cpu_name + ".txt");
            ptrace.push_back(power_trace);
        }

        cout << "Creating Scheduler and kernel" << endl;
        EDFScheduler edfsched;
        MRTKernel kern(&edfsched, cpuFactory, 8);

        /* ------------------------- Creating task -------------------------*/
        for (unsigned int i=0; i<8; ++i) {
            string task_name = "Task_" + to_string(i);
            cout << "Creating task: " << task_name << endl;

            PeriodicTask *t = new PeriodicTask(200, 200, 50, task_name);
            t->insertCode("fixed(100,bzip2);");

            ttrace.attachToTask(*t);
            jtrace.attachToTask(*t);

            kern.addTask(*t, "");
        }

        cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        cout << "Running simulation!" << endl;

        SIMUL.run(5000);
    } catch (BaseExc &e) {
        cout << e.what() << endl;
    }
}
