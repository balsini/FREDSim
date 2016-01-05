/*
  In this example, a simple system is simulated, consisting of two
  real-time tasks scheduled by EDF on a single processor.
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

int main()
{
    try {

        SIMUL.dbg.enable("All");
        SIMUL.dbg.setStream("debug.txt");
        // set the trace file. This can be visualized by the
        // rttracer tool
        //JavaTrace jtrace("trace.trc");

        TextTrace ttrace("trace.txt");
        JSONTrace jtrace("trace.json");
  
        cout << "Creating Scheduler and kernel" << endl;
        EDFScheduler edfsched;
        RTKernel kern(&edfsched);

        cout << "Creating the first task" << endl;
        PeriodicTask t1(4, 4, 0, "Task0");

        cout << "Inserting code" << endl;
        t1.insertCode("fixed(2);");
        //t1.setAbort(false);


        cout << "Creating the second task" << endl;
        PeriodicTask t2(5, 5, 0, "Task1");

        cout << "Inserting code" << endl;
        t2.insertCode("fixed(2);");
        //t2.setAbort(false);

        cout << "Creating the third task" << endl;
        PeriodicTask t3(6, 6, 0, "Task2");
        cout << "Inserting code" << endl;
        t3.insertCode("fixed(2);");
        //t3.setAbort(false);


        cout << "Setting up traces" << endl;
	
        // new way
        ttrace.attachToTask(&t1);
        ttrace.attachToTask(&t2);
        ttrace.attachToTask(&t3);

        jtrace.attachToTask(&t1);
        jtrace.attachToTask(&t2);
        jtrace.attachToTask(&t3);

        cout << "Adding tasks to schedulers" << endl;

        kern.addTask(t1, "");
        kern.addTask(t2, "");
        kern.addTask(t3, "");
//        kern.addTask(t4, "");
  
        cout << "Ready to run!" << endl;
        // run the simulation for 500 units of time
        SIMUL.run(500);
    } catch (BaseExc &e) {
        cout << e.what() << endl;
    }
}
