/*
  In this example, a simple system is simulated, to provide
  an evaluation of different workloads running on an big.LITTLE
  Odroid-XU3 embedded board.
*/

#include <kernel.hpp>
#include <edfsched.hpp>
#include <jtrace.hpp>
#include <texttrace.hpp>
#include <json_trace.hpp>
#include <rttask.hpp>
#include <instr.hpp>

using namespace MetaSim;
using namespace RTSim;

const int SIMUL_DURATION = 500;

int main(int argc, char *argv[])
{
    try {
        SIMUL.dbg.enable("All");
        SIMUL.dbg.setStream("debug.txt");

        TextTrace ttrace("trace.txt");
        JSONTrace jtrace("trace.json");

        cout << "Creating Scheduler and kernel" << endl;
        EDFScheduler edfsched;
        RTKernel kern(&edfsched);

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
