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
    arch.TASK_APPENDED = 0;

    arch.PERIOD_MIN = 200000;
    arch.PERIOD_MAX = 500000;
    arch.PERIOD_break_list.push_back(300000);

    arch.A_TOT = 1000000;
    arch.RHO = 10;

    arch.PARTITION_NUM = 2;
    arch.SLOT_NUM_MIN = 2;
    arch.SLOT_NUM_MAX = 2;

    arch.PERIOD_bucket = {200000,
                          205000,
                          210000,
                          215000,
                          220000,
                          225000,
                          230000,
                          235000,
                          240000,
                          245000,
                          250000,
                          255000,
                          260000,
                          265000,
                          400000,
                          405000,
                          410000,
                          415000,
                          420000,
                          425000,
                          430000,
                          435000,
                          440000,
                          445000,
                          450000,
                          455000,
                          460000,
                          465000};
    arch.SPEEDUP = 1;

    arch.U_SW = 0.3;
    arch.C_SW_MIN = 1;

    arch.U_HW = 0.6;
    arch.U_HW_UB = 0.95;

    arch.FRI = TB_PREEMPTIVE_B;


    Environment_details_t ed = generateEnvironment(arch);

    extraTasks_details_t extra_details;
    extra_details.N = 2;
    extra_details.U_SW = 0.1;
    extra_details.U_HW = 0.2;

    Environment_details_t ed_new;

    ed_new = environmentAddTask(ed, arch, extra_details);

    printEnvironment(ed);
    printEnvironment(ed_new);
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
