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

const unsigned int SW_TASK_NUM = 3;

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


    StatMax soft_rt_max[SW_TASK_NUM];
    StatMean soft_rt_mean[SW_TASK_NUM];
    StatMin soft_rt_min[SW_TASK_NUM];

    StatMax hard_rt_max[SW_TASK_NUM];
    StatMean hard_rt_mean[SW_TASK_NUM];
    StatMin hard_rt_min[SW_TASK_NUM];


    cout << "Creating scheduler and kernel" << endl;
    FPScheduler softSched;
    RTKernel kern(&softSched);

    //FCFSResManager rm("ResourceManager");
    //kern.setResManager(&rm);
    //rm.addResource("res1");

    FPGAKernel FPGA_real(DISPATCHER_FIRST);
    //FPGAKernel FPGA_real(DISPATCHER_MORE_FREE_SLOTS);

    // Creating partitions
    vector<Scheduler *>partition;
    partition.push_back(FPGA_real.addPartition(1));
    partition.push_back(FPGA_real.addPartition(1));
    ////////////////////

    //Resource res1("res1");



    cout << "Creating tasks" << endl;

    vector<AcceleratedTask *> sw_tasks;

    AcceleratedTask t0(10, 10, 0, "Task0");
    AcceleratedTask t1(11, 11, 0, "Task1");
    AcceleratedTask t2(20, 20, 0, "Task2");

    sw_tasks.push_back(&t0);
    sw_tasks.push_back(&t1);
    sw_tasks.push_back(&t2);

    cout << "Inserting codes" << endl;

    t0.insertCode("fixed(1);accelerate(4);fixed(2);");
    t1.insertCode("fixed(1);accelerate(4);fixed(2);");
    t2.insertCode("fixed(1);accelerate(4);fixed(2);");

    cout << "Setting hardware tasks affinities" << endl;

    vector<Scheduler *> affinity;

    affinity = {partition.at(0)};
    t0.getHW()->setAffinity(affinity);
    affinity = {partition.at(0), partition.at(1)};
    t1.getHW()->setAffinity(affinity);
    affinity = {partition.at(0), partition.at(1)};
    t2.getHW()->setAffinity(affinity);

    cout << "Linking statistics accumulators" << endl;

    for (unsigned int i=0; i<SW_TASK_NUM; ++i) {
      sw_tasks.at(i)->addMaxRTStat(&soft_rt_max[i]);
      sw_tasks.at(i)->addMeanRTStat(&soft_rt_mean[i]);
      sw_tasks.at(i)->addMinRTStat(&soft_rt_min[i]);

      sw_tasks.at(i)->getHW()->addMaxRTStat(&hard_rt_max[i]);
      sw_tasks.at(i)->getHW()->addMeanRTStat(&hard_rt_mean[i]);
      sw_tasks.at(i)->getHW()->addMinRTStat(&hard_rt_min[i]);
    }

    cout << "Setting up traces" << endl;

    for (unsigned int i=0; i<SW_TASK_NUM; ++i) {
      ttrace.attachToTask(sw_tasks.at(i));
      pstrace.attachToTask(sw_tasks.at(i));
      pstrace.attachToTask(sw_tasks.at(i)->getHW());
    }

    cout << "Adding tasks to schedulers" << endl;

    kern.addTask(t0, "1");
    kern.addTask(t1, "2");
    kern.addTask(t2, "3");

    for (unsigned int i=0; i<SW_TASK_NUM; ++i) {
      FPGA_real.addTask(*(sw_tasks.at(i)->getHW()), "");
    }

    cout << "Ready to run!" << endl;
    SIMUL.run(10000);

    cout << endl << "#####################" << endl;
    cout << "SOFTWARE TASKS\n" << endl;

    for (unsigned int i=0; i<SW_TASK_NUM; ++i) {
    cout << i << " :" << '\t';
    cout << "Lateness (min - mean - max):\t"
         << soft_rt_min[i].getValue() << '\t'
         << soft_rt_mean[i].getValue() << '\t'
         << soft_rt_max[i].getValue() << endl;
    }

    cout << "\nHARDWARE TASKS\n" << endl;

    for (unsigned int i=0; i<SW_TASK_NUM; ++i) {
    cout << i << " :" << '\t';
    cout << "Lateness (min - mean - max):\t"
         << hard_rt_min[i].getValue() << '\t'
         << hard_rt_mean[i].getValue() << '\t'
         << hard_rt_max[i].getValue() << endl;
    }
    cout << "#####################" << endl << endl;
  } catch (BaseExc &e) {
    cout << e.what() << endl;
  }
}
