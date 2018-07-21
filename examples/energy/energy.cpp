/*
  In this example, a simple system is simulated, to provide
  an evaluation of different workloads running on an big.LITTLE
  Odroid-XU3 embedded board.
*/

#include <mrtkernel.hpp>
#include <cpu.hpp>
#include <edfsched.hpp>
#include <jtrace.hpp>
#include <texttrace.hpp>
#include <json_trace.hpp>
#include <rttask.hpp>
#include <instr.hpp>

using namespace MetaSim;
using namespace RTSim;

const int SIMUL_DURATION = 5000;

int main()
{
    try {
        SIMUL.dbg.enable("All");
        SIMUL.dbg.setStream("debug.txt");

        TextTrace ttrace("trace.txt");
        JSONTrace jtrace("trace.json");

        cout << "Creating Scheduler and kernel" << endl;
        EDFScheduler edfsched;
        customCPUFactory *cpuFactory = new customCPUFactory();

        for (unsigned int i=0; i<4; ++i) {
            /* Create LITTLE CPUs */
            vector<double> V =
            {
                0.92, 0.919643, 0.919357, 0.918924, 0.95625, 0.9925, 1.02993, 1.0475, 1.08445, 1.12125, 1.15779, 1.2075, 1.25625
            };
            vector<unsigned int> F =
            {
                200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400
            };
            CPU *c = new CPU("LITTLE_" + std::to_string(i), V, F);
            cpuFactory->addCPU(c);
        }

        for (unsigned int i=0; i<4; ++i) {
            /* Create big CPUs */
            vector<double> V =
            {
                0.916319, 0.915475, 0.915102, 0.91498, 0.91502, 0.90375, 0.916562, 0.942543, 0.96877, 0.994941, 1.02094, 1.04648, 1.05995, 1.08583, 1.12384, 1.16325, 1.20235, 1.2538, 1.33287
            };
            vector<unsigned int> F =
            {
                200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000
            };
            CPU *c = new CPU("big_" + std::to_string(i), V, F);
            cpuFactory->addCPU(c);
        }

        MRTKernel kern(&edfsched, cpuFactory, 8);

        cout << "Creating the first task" << endl;
        PeriodicTask t1(4, 4, 0, "Task0");
        cout << "Inserting code" << endl;
        t1.insertCode("fixed(2);");

        cout << "Setting up traces" << endl;

        ttrace.attachToTask(t1);
        jtrace.attachToTask(t1);

        cout << "Adding tasks to schedulers" << endl;
        kern.addTask(t1, "");

        cout << "Running simulation!" << endl;
        // run the simulation for SIMUL_DURATION units of time
        SIMUL.run(SIMUL_DURATION);
    } catch (BaseExc &e) {
        cout << e.what() << endl;
    }
}
