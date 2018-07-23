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

vector<double> splitd(const string &strToSplit, char delimeter)
{
    stringstream ss(strToSplit);
    string item;
    vector<double> splittedStrings;
    while (getline(ss, item, delimeter))
       splittedStrings.push_back(stod(item));
    return splittedStrings;
}

int main(int argc, char *argv[])
{
    vector<double> opp;
    vector<double> k0;
    vector<double> kw;

    opp = splitd(argv[1], ',');
    k0 = splitd(argv[2], ',');
    kw = splitd(argv[3], ',');

    if (k0.size() != 8 || kw.size() != 8)
        exit(-1);

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

            PowerModel *pm = new PowerModelBP(0, 0, 0, 0, k0[i]);
            CPU *c = new CPU(cpu_name, V, F, pm, k0[i]);
            c->setOPP(opp[i]);
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

            PowerModel *pm = new PowerModelBP(0, 0, 0, 0, k0[i + 4]);
            CPU *c = new CPU(cpu_name, V, F, pm, k0[i + 4]);
            c->setOPP(opp[i + 4]);
            cpuFactory->addCPU(c);
            TracePowerConsumption *power_trace = new TracePowerConsumption(c, 10, "power_" + cpu_name + ".txt");
            ptrace.push_back(power_trace);
        }

        cout << "Creating Scheduler and kernel" << endl;
        EDFScheduler edfsched;
        MRTKernel kern(&edfsched, cpuFactory, 1);

        /* ------------------------- Creating task -------------------------*/
        string task_name = "Task";
        cout << "Creating task: " << task_name << endl;

        PeriodicTask *t = new PeriodicTask(4, 4, 0, task_name);
        t->insertCode("fixed(1,2);");

        ttrace.attachToTask(*t);
        jtrace.attachToTask(*t);

        kern.addTask(*t, "");

        cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        cout << "Running simulation!" << endl;

        SIMUL.run(5000);
    } catch (BaseExc &e) {
        cout << e.what() << endl;
    }
}
