#include <ctime>
#include <cstdlib>
#include <vector>
#include <iostream>

#include "generator.h"

int main()
{
  try {
    srand(time(0));

    overallArchitecture_t arch;

    arch.TASK_MAX_K = 1;

    arch.PERIOD_MIN = 200;
    arch.PERIOD_MAX = 500;
    arch.PERIOD_break_list.push_back(300);

    arch.A_TOT = 1000;
    arch.RHO = 10;

    arch.PARTITION_NUM = 2;
    arch.SLOT_NUM_MIN = 2;
    arch.SLOT_NUM_MAX = 2;

    arch.PERIOD_bucket = {215,230,243,350,367,452};
    arch.SPEEDUP = 1;

    arch.U_SW = 0.1;
    arch.C_SW_MIN = 1;

    arch.U_HW = 0.6;
    arch.U_HW_UB = 0.95;

    arch.FRI = TB_PREEMPTIVE_B;


    Environment_details_t ed = generateEnvironment(arch);

    printEnvironment(ed);
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
