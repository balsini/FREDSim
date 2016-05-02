#include "generator.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <iostream>


double UniformVarRand::get()
{
  double f = (double)rand() / RAND_MAX;

  return _min + f * (_max - _min);
}

template <class T>
T BucketVar<T>::get(T min, T max)
{
  if (min > max)
    throw architectureException("BucketVar: min > max");

  T retValue;

  typename std::vector<T>::iterator it;
  for (it = _bucket.begin(); it != _bucket.end(); ++it) {
    if (*it <= max && *it >= min) {
      retValue = *it;
      _bucket.erase(it);

      return retValue;
    }
  }

  throw architectureException("No available values in bucket can satisfy the requirement");
}

void environmentInit(Environment_details_t &e)
{
  e.A_TOT = 0;
  e.FRI = TB_NONPREEMPTIVE_B;
  e.N_S = 0;
  e.P = 0;
  e.partition_slot_size.clear();
  e.rho = 0;
  e.slots_per_partition.clear();
  e.speedup = 0;
  e.tasks_number = 0;
  e.task_per_partition.clear();
  e.taskset_U_SW = 0;
  e.taskset_U_HW = 0;
}


void taskInit(task_details_t &t)
{
  t.A = 0;
  t.C_HW = 0;
  t.C_SW_1 = 0;
  t.C_SW_2 = 0;
  t.D = 0;
  t.P = 0;
  t.T = 0;
  t.U = 0;
}


std::vector<double> UUnifastWorker(int number, double MYU, double UB)
{
  std::vector<double> result;
  double sumU = MYU;
  double UMin = 0.005;
  double NextSumU, base, exp, temp;
  UniformVarRand myvar(0, 1);
  for (int i = 0; i < number - 1; i++){
    base = myvar.get();
    exp = (double)((double)1 / (double)(number - i - 1));
    temp = pow(base, exp);
    NextSumU = sumU*temp;
    if (NextSumU>sumU - UMin)
      NextSumU = sumU - UMin;
    if (NextSumU<(number - i - 1)*UMin)
      NextSumU = (number - i - 1)*UMin;

    result.push_back(sumU - NextSumU);
    sumU = NextSumU;
  }
  result.push_back(sumU);
  return result;
}

std::vector<double> UUnifast(int number, double MYU, double UB)
{
  std::vector<double>ret;
  bool go_on = true;

  if (MYU >= (UB * number - 0.00000001)) {
    for (unsigned int i=0; i<number; ++i)
      ret.push_back(UB);

    return ret;
  }

  while (go_on) {
    go_on = false;
    ret = UUnifastWorker(number, MYU, UB);
    for (unsigned int i=0; i<ret.size(); ++i) {
      if (ret.at(i) > UB) {
        go_on = true;
        break;
      }
    }
  }
  return ret;
}


void verifyEnvironment(const overallArchitecture_t &arch)
{
  if (arch.PARTITION_NUM != (arch.PERIOD_break_list.size() + 1)) {
    throw architectureException("Wrong PERIOD_break_list size. Please insert a number of period breaks equal to PARTITION_NUM-1");
  }

  if (arch.U_HW_UB >= 1) {
    throw architectureException("Deadlock due to too high U_HW_UB, please choose a number smaller than 1");
  }

  if (arch.TASK_MAX_K <= 0) {
    throw architectureException("TASK_MAX_K must be greater than zero");
  }

  if (arch.U_HW >= 1) {
    throw architectureException("Deadlock due to too high U_HW, please choose a number smaller than 1");
  }
}

Environment_details_t environmentAddTask(const Environment_details_t &env, const overallArchitecture_t &arch, const extraTasks_details_t &t)
{
  Environment_details_t e = env;

  std::vector<unsigned int> periods;

  e.tasks_number += t.N;

  std::vector<task_details_t> new_tasks;

  for (unsigned int i=0; i<t.N; ++i) {
    task_details_t task;

    task.A = i % e.P;
    task.U = t.U_SW;

    unsigned int P_break_min, P_break_max;
    if (task.A==0) {
      P_break_min = arch.PERIOD_MIN;
    } else {
      P_break_min = arch.PERIOD_break_list.at(task.A-1);
    }

    if (task.A==(e.task_per_partition.size()-1)) {
      P_break_max = arch.PERIOD_MAX;
    } else {
      P_break_max = arch.PERIOD_break_list.at(task.A);
    }

    task.T = e.bucket.get(P_break_min, P_break_max);
    task.D = task.T;

    task.C_HW = task.T * e.U_HW;


    unsigned int C = task.U * task.T;

    if (C < (2 * arch.C_SW_MIN))
      throw architectureException("environmentAddTask: C (" + patch::to_string(C) + ") too small with respect to C_SW_MIN (" + patch::to_string(arch.C_SW_MIN) + ")");

    UniformVarRand tasksCi(arch.C_SW_MIN, C - arch.C_SW_MIN);
    task.C_SW_1 = tasksCi.get();
    task.C_SW_2 = C - task.C_SW_1;

    periods.push_back(task.T);

    e.task_per_partition.at(task.A).push_back(task);
  }


  for (unsigned int p=0; p<e.task_per_partition.size(); ++p) {
    for (unsigned int t=0; t<e.task_per_partition.at(p).size(); ++t) {
      periods.push_back(e.task_per_partition.at(p).at(t).T);
    }
  }

  // Make tasks Rate Monotonic
  sort(periods.begin(), periods.end());
  for (unsigned int p=0; p<e.task_per_partition.size(); ++p) {
    for (unsigned int t=0; t<e.task_per_partition.at(p).size(); ++t) {
      auto it = std::find(periods.begin(), periods.end(), e.task_per_partition.at(p).at(t).T);
      if (it == periods.end()) {
        e.task_per_partition.at(p).at(t).P = -1;
      } else {
        e.task_per_partition.at(p).at(t).P = std::distance(periods.begin(), it);
      }
    }
  }

  return e;
}

Environment_details_t generateEnvironment(const overallArchitecture_t &arch)
{
  verifyEnvironment(arch);

  Environment_details_t e;
  environmentInit(e);

  e.A_TOT = arch.A_TOT;
  e.rho = arch.RHO;
  e.speedup = arch.SPEEDUP;
  e.FRI = arch.FRI;
  e.U_HW = arch.U_HW;
  e.U_HW_UB = arch.U_HW_UB;

  //////////////////////////
  // Partitions and Slots //
  //////////////////////////

  e.P = arch.PARTITION_NUM;

  UniformVarRand slotsRand(arch.SLOT_NUM_MIN, arch.SLOT_NUM_MAX);

  unsigned biggestPartitionSlots = 0;
  e.N_S = 0; // Total number of slots
  for (unsigned int i=0; i<e.P; ++i) {
    unsigned int N_S_i = slotsRand.get(); // Number of slots for partition i
    e.N_S += N_S_i;
    e.slots_per_partition.push_back(N_S_i);

    unsigned int slot_size = e.A_TOT / (e.P * N_S_i);
    e.partition_slot_size.push_back(slot_size);

    e.partition_U.push_back(e.U_HW * N_S_i);

    if (N_S_i > biggestPartitionSlots)
      biggestPartitionSlots = N_S_i;
  }

  ///////////
  // Tasks //
  ///////////

  // For each partition, the minimum number of tasks must be greater than
  // the number of slots.

  // Create an uninitialized taskset for each partition
  e.tasks_number = 0;
  unsigned int appended_per_partition = arch.TASK_APPENDED / e.P;
  unsigned int appended_remaining = arch.TASK_APPENDED % e.P;

  for (unsigned int i=0; i<e.P; ++i) {

    UniformVarRand tasksRand(e.slots_per_partition.at(i),
                             e.slots_per_partition.at(i) * arch.TASK_MAX_K);

    std::vector<task_details_t> partition_taskset;

    unsigned int partition_taskset_size = tasksRand.get() + appended_per_partition;
    if (appended_remaining) {
      appended_remaining--;
      partition_taskset_size++;
    }

    e.tasks_number += partition_taskset_size;

    for (unsigned t = 0; t<partition_taskset_size; ++t) {
      task_details_t td;
      taskInit(td);
      partition_taskset.push_back(td);
    }

    e.task_per_partition.push_back(partition_taskset);
  }

  e.taskset_U_SW = arch.U_SW;
  e.taskset_U_HW = arch.U_HW;
  std::vector<double> utilization_factors = UUnifast(e.tasks_number,
                                                     e.taskset_U_SW,
                                                     1.0);

  // Generate the periods for each partition
  e.bucket.set(arch.PERIOD_bucket);

  std::vector<unsigned int> periods;
  unsigned int uf_i = 0;
  for (unsigned int p=0; p<e.task_per_partition.size(); ++p) {

    // Generates the utilization factors for each hardware task
    std::vector<double> utilization_factors_hw = UUnifast(e.task_per_partition.at(p).size(),
                                                          e.partition_U.at(p),
                                                          e.U_HW_UB);

    unsigned int P_break_min, P_break_max;
    if (p==0) {
      P_break_min = arch.PERIOD_MIN;
    } else {
      P_break_min = arch.PERIOD_break_list.at(p-1);
    }

    if (p==(e.task_per_partition.size()-1)) {
      P_break_max = arch.PERIOD_MAX;
    } else {
      P_break_max = arch.PERIOD_break_list.at(p);
    }

    // Assign tasks parameters
    for (unsigned int t=0; t<e.task_per_partition.at(p).size(); ++t) {
      e.task_per_partition.at(p).at(t).A = p;
      e.task_per_partition.at(p).at(t).U = utilization_factors.at(uf_i);
      e.task_per_partition.at(p).at(t).T = e.bucket.get(P_break_min, P_break_max);

      periods.push_back(e.task_per_partition.at(p).at(t).T);
      e.task_per_partition.at(p).at(t).D = e.task_per_partition.at(p).at(t).T;

      unsigned int C = e.task_per_partition.at(p).at(t).U * e.task_per_partition.at(p).at(t).T;

      if (C < (2 * arch.C_SW_MIN))
        throw architectureException("generateEnvironment: C (" + patch::to_string(C) + ") too small with respect to C_SW_MIN (" + patch::to_string(arch.C_SW_MIN) + ")");

      UniformVarRand tasksCi(arch.C_SW_MIN, C - arch.C_SW_MIN);
      unsigned int C1 = tasksCi.get();
      unsigned int C2 = C - C1;
      e.task_per_partition.at(p).at(t).C_SW_1 = C1;
      e.task_per_partition.at(p).at(t).C_SW_2 = C2;

      // C = U * T
      e.task_per_partition.at(p).at(t).C_HW = utilization_factors_hw.at(t) *
          e.task_per_partition.at(p).at(t).T;

      uf_i++;
    }
  }

  // Make tasks Rate Monotonic
  sort(periods.begin(), periods.end());
  for (unsigned int p=0; p<e.task_per_partition.size(); ++p) {
    for (unsigned int t=0; t<e.task_per_partition.at(p).size(); ++t) {
      auto it = std::find(periods.begin(), periods.end(), e.task_per_partition.at(p).at(t).T);
      if (it == periods.end()) {
        e.task_per_partition.at(p).at(t).P = -1;
      } else {
        e.task_per_partition.at(p).at(t).P = std::distance(periods.begin(), it);
      }
    }
  }

  return e;
}


void printEnvironment(const Environment_details_t &e)
{
  std::cout << "ReconfigurableArea\t" << e.A_TOT << std::endl;
  std::cout << "Speedup\t" << e.speedup << std::endl;
  std::cout << "Rho\t" << e.rho << std::endl;
  std::cout << "U_HW\t" << e.U_HW<< std::endl;
  std::cout << "U_HW_UB\t" << e.U_HW_UB << std::endl;
  std::cout << "Partition\t" << e.P << std::endl;
  std::cout << "TotalSlots\t" << e.N_S << std::endl;

  for (unsigned int i=0; i<e.slots_per_partition.size(); ++i) {
    std::cout << "Partition[" << i << "]:" << std::endl;
    std::cout << "\tSlots\t" << e.slots_per_partition.at(i) << std::endl;
    std::cout << "\tSlotSize\t" << e.partition_slot_size.at(i) << std::endl;
    std::cout << "\tU\t" << e.partition_U.at(i) << std::endl;

    for (unsigned int j=0; j<e.task_per_partition.at(i).size(); ++j) {
      std::cout << "\tTask[" << j << "]:" << std::endl;
      std::cout << "\t\tU\t" << e.task_per_partition.at(i).at(j).U << std::endl;
      std::cout << "\t\tT\t" << e.task_per_partition.at(i).at(j).T << std::endl;
      std::cout << "\t\tD\t" << e.task_per_partition.at(i).at(j).D << std::endl;
      std::cout << "\t\tPrio\t" << e.task_per_partition.at(i).at(j).P << std::endl;
      std::cout << "\t\tC_1\t" << e.task_per_partition.at(i).at(j).C_SW_1 << std::endl;
      std::cout << "\t\tC_2\t" << e.task_per_partition.at(i).at(j).C_SW_2 << std::endl;
      std::cout << "\t\tC_HW\t" << e.task_per_partition.at(i).at(j).C_HW << std::endl;
      std::cout << "\t\tr\t" << (double)e.partition_slot_size.at(i)/e.rho << std::endl;
    }
  }
}
