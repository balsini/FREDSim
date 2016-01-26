/*
  In this example, a simple system is simulated, consisting of two
  real-time tasks scheduled by EDF on a single processor.
*/
#include <kernel.hpp>
#include <task.hpp>
#include <edfsched.hpp>
#include <fifosched.hpp>
#include <jtrace.hpp>
#include <texttrace.hpp>
#include <json_trace.hpp>
#include <ps_trace.hpp>
#include <acceleratedtask.hpp>
#include <instr.hpp>
#include <fcfsresmanager.hpp>
#include <resource.hpp>
#include <basestat.hpp>

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
    PSTrace pstrace("trace.pst");

    StatMax soft_rt_max1("soft_rt_max1");
    StatMean soft_rt_mean1("soft_rt_mean1");
    StatMin soft_rt_min1("soft_rt_min1");
    StatMax soft_rt_max2("soft_rt_max2");
    StatMean soft_rt_mean2("soft_rt_mean2");
    StatMin soft_rt_min2("soft_rt_min2");

    cout << "Creating Scheduler and kernel" << endl;
    EDFScheduler edfsched;
    FCFSResManager rm("ResourceManager");
    RTKernel kern(&edfsched);
    kern.setResManager(&rm);

    FIFOScheduler fifosched;
    RTKernel FPGA(&fifosched);


    //Resource res1("res1");
    rm.addResource("res1");



    cout << "Creating the first task" << endl;
    AcceleratedTask t1(10, 10, 0, "Task1");
    cout << "Inserting code" << endl;
    t1.insertCode("fixed(2);accelerate(3);fixed(1);");
    //t1.setAbort(false);
    t1.addMaxRTStat(&soft_rt_max1);
    t1.addMeanRTStat(&soft_rt_mean1);
    t1.addMinRTStat(&soft_rt_min1);


    cout << "Creating the second task" << endl;
    AcceleratedTask t2(11, 11, 0, "Task2");
    cout << "Inserting code" << endl;
    t2.insertCode("fixed(2);accelerate(5);fixed(2);");
    //t2.setAbort(false);
    t2.addMaxRTStat(&soft_rt_max2);
    t2.addMeanRTStat(&soft_rt_mean2);
    t2.addMinRTStat(&soft_rt_min2);

    /*
        cout << "Creating the third task" << endl;
        AcceleratedTask t3(6, 6, 0, "Task2");

        cout << "Inserting code" << endl;
        t3.insertCode("fixed(2);");
        //t3.setAbort(false);
*/


    cout << "Setting up traces" << endl;

    // new way
    ttrace.attachToTask(&t1);
    ttrace.attachToTask(&t2);
    //        ttrace.attachToTask(&t3);

    jtrace.attachToTask(&t1);
    jtrace.attachToTask(&t2);
    //        jtrace.attachToTask(&t3);

    pstrace.attachToTask(&t1);
    pstrace.attachToTask(&t2);
    pstrace.attachToTask(t1.getHW());
    pstrace.attachToTask(t2.getHW());
    //        pstrace.attachToTask(&t3);

    cout << "Adding tasks to schedulers" << endl;

    kern.addTask(t1, "");
    kern.addTask(t2, "");
    FPGA.addTask(*(t1.getHW()), "");
    FPGA.addTask(*(t2.getHW()), "");
    //        kern.addTask(t3, "");

    cout << "Ready to run!" << endl;
    // run the simulation for 500 units of time
    SIMUL.run(500);

    cout << endl << "#####################" << endl;
    cout << "SOFTWARE TASKS" << endl;
    cout << "\nTask1" << endl;
    cout << "Maximum lateness:\t" << soft_rt_max1.getValue() << endl;
    cout << "Mean lateness:\t" << soft_rt_mean1.getValue() << endl;
    cout << "Minimum lateness:\t" << soft_rt_min1.getValue() << endl;
    cout << "\nTask2" << endl;
    cout << "Maximum lateness:\t" << soft_rt_max2.getValue() << endl;
    cout << "Mean lateness:\t" << soft_rt_mean2.getValue() << endl;
    cout << "Minimum lateness:\t" << soft_rt_min2.getValue() << endl;
    cout << "#####################" << endl << endl;
  } catch (BaseExc &e) {
    cout << e.what() << endl;
  }
}
