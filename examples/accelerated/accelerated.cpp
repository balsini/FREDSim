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

#include <vector>

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

    StatMax soft_rt_max3("soft_rt_max3");
    StatMean soft_rt_mean3("soft_rt_mean3");
    StatMin soft_rt_min3("soft_rt_min3");

    cout << "Creating Scheduler and kernel" << endl;
    FPScheduler softSched;
    RTKernel kern(&softSched);

    //FCFSResManager rm("ResourceManager");
    //kern.setResManager(&rm);
    //rm.addResource("res1");

    FPGAKernel FPGA_real;

    // Creating partitions
    vector<Scheduler *>partition;
    partition.push_back(FPGA_real.addPartition(1));
    partition.push_back(FPGA_real.addPartition(1));
    ////////////////////

    //RTKernel FPGA(&hardsched);
    //Resource res1("res1");



    cout << "Creating the first task" << endl;
    AcceleratedTask t1(10, 10, 0, "Task1");
    cout << "Inserting code" << endl;
    t1.insertCode("fixed(2);accelerate(2);fixed(1);");

    vector<Scheduler *> t1_affinity = {partition.at(0)};
    t1.getHW()->setAffinity(t1_affinity);
    //t1.setAbort(false);
    t1.addMaxRTStat(&soft_rt_max1);
    t1.addMeanRTStat(&soft_rt_mean1);
    t1.addMinRTStat(&soft_rt_min1);


    cout << "Creating the second task" << endl;
    AcceleratedTask t2(11, 11, 0, "Task2");
    cout << "Inserting code" << endl;
    t2.insertCode("fixed(1);accelerate(2);fixed(2);");

    vector<Scheduler *> t2_affinity = {partition.at(1)};
    t2.getHW()->setAffinity(t2_affinity);
    //t2.setAbort(false);
    t2.addMaxRTStat(&soft_rt_max2);
    t2.addMeanRTStat(&soft_rt_mean2);
    t2.addMinRTStat(&soft_rt_min2);

    cout << "Creating the third task" << endl;
    AcceleratedTask t3(19, 19, 0, "Task3");
    cout << "Inserting code" << endl;
    t3.insertCode("fixed(2);accelerate(4);fixed(2);");

    vector<Scheduler *> t3_affinity = {partition.at(0), partition.at(1)};
    t3.getHW()->setAffinity(t3_affinity);
    //t2.setAbort(false);
    t3.addMaxRTStat(&soft_rt_max3);
    t3.addMeanRTStat(&soft_rt_mean3);
    t3.addMinRTStat(&soft_rt_min3);

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
    ttrace.attachToTask(&t3);

    jtrace.attachToTask(&t1);
    jtrace.attachToTask(&t2);
    jtrace.attachToTask(&t3);

    pstrace.attachToTask(&t1);
    pstrace.attachToTask(&t2);
    pstrace.attachToTask(&t3);
    pstrace.attachToTask(t1.getHW());
    pstrace.attachToTask(t2.getHW());
    pstrace.attachToTask(t3.getHW());

    cout << "Adding tasks to schedulers" << endl;

    kern.addTask(t1, "1");
    kern.addTask(t2, "2");
    kern.addTask(t3, "3");
    FPGA_real.addTask(*(t1.getHW()), "");
    FPGA_real.addTask(*(t2.getHW()), "");
    FPGA_real.addTask(*(t3.getHW()), "");
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
    cout << "\nTask3" << endl;
    cout << "Maximum lateness:\t" << soft_rt_max3.getValue() << endl;
    cout << "Mean lateness:\t" << soft_rt_mean3.getValue() << endl;
    cout << "Minimum lateness:\t" << soft_rt_min3.getValue() << endl;
    cout << "#####################" << endl << endl;
  } catch (BaseExc &e) {
    cout << e.what() << endl;
  }
}
