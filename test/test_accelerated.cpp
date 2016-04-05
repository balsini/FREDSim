#include "catch.hpp"

#include <metasim.hpp>
#include <rttask.hpp>
#include <acceleratedtask.hpp>
#include <kernel.hpp>
#include <fpgakernel.hpp>
#include <fpsched.hpp>
#include <basestat.hpp>

#include <ps_trace.hpp>

#include <vector>
#include <string>

using namespace MetaSim;
using namespace RTSim;

const unsigned int SW_TASK_NUM = 10;
const unsigned int PARTITION_NUM = 3;
const unsigned int SLOT_NUM = 2;

TEST_CASE("Accelerated multiple runs equal results")
{
  PSTrace pstrace("trace.pst");

  StatMax soft_rt_max[SW_TASK_NUM];
  StatMean soft_rt_mean[SW_TASK_NUM];
  StatMin soft_rt_min[SW_TASK_NUM];

  StatMax hard_rt_max[SW_TASK_NUM];
  StatMean hard_rt_mean[SW_TASK_NUM];
  StatMin hard_rt_min[SW_TASK_NUM];

  FPScheduler softSched;
  RTKernel kern(&softSched);

  FPGAKernel FPGA_real(DISPATCHER_MORE_FREE_SLOTS, FP_NONPREEMPTIVE);

  vector<Scheduler *>partition;
  for (unsigned int i=0; i<PARTITION_NUM; ++i)
    partition.push_back(FPGA_real.addPartition(SLOT_NUM));

  AcceleratedTask * at[SW_TASK_NUM];
  string task_name = "Task_";
  for (unsigned int i=0; i<SW_TASK_NUM; ++i) {
    int period = 10 + i;
    at[i] = new AcceleratedTask(period, period, 0, task_name + to_string(i));
    at[i]->insertCode("fixed(1);accelerate(4);fixed(1);");
    at[i]->getHW()->setConfigurationTime(2);

    kern.addTask(*(at[i]), to_string(i));
    FPGA_real.addTask(*(at[i]->getHW()), to_string(i));


    vector<Scheduler *> affinity;
    affinity = {partition.at(i % PARTITION_NUM)};
    at[i]->getHW()->setAffinity(affinity);

    pstrace.attachToTask(at[i]);
    pstrace.attachToTask(at[i]->getHW());


    at[i]->addMaxRTStat(&soft_rt_max[i]);
    at[i]->addMeanRTStat(&soft_rt_mean[i]);
    at[i]->addMinRTStat(&soft_rt_min[i]);

    at[i]->getHW()->addMaxRTStat(&hard_rt_max[i]);
    at[i]->getHW()->addMeanRTStat(&hard_rt_mean[i]);
    at[i]->getHW()->addMinRTStat(&hard_rt_min[i]);
  }

  double old_val[SW_TASK_NUM][6];
  for (unsigned int i=0; i<10; i++) {
    SIMUL.run(5000);

    if (i > 0) {
      for (unsigned j=0; j<SW_TASK_NUM; j++) {
        REQUIRE(old_val[j][0] == soft_rt_min[j].getValue());
        REQUIRE(old_val[j][1] == soft_rt_mean[j].getValue());
        REQUIRE(old_val[j][2] == soft_rt_max[j].getValue());
        REQUIRE(old_val[j][3] == hard_rt_min[j].getValue());
        REQUIRE(old_val[j][4] == hard_rt_mean[j].getValue());
        REQUIRE(old_val[j][5] == hard_rt_max[j].getValue());
      }
    }

    for (unsigned j=0; j<SW_TASK_NUM; j++) {
      old_val[j][0] = soft_rt_min[j].getValue();
      old_val[j][1] = soft_rt_mean[j].getValue();
      old_val[j][2] = soft_rt_max[j].getValue();
      old_val[j][3] = hard_rt_min[j].getValue();
      old_val[j][4] = hard_rt_mean[j].getValue();
      old_val[j][5] = hard_rt_max[j].getValue();
    }
  }
}
