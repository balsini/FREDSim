#ifndef __FPGAKERNEL_HPP__
#define __FPGAKERNEL_HPP__

#include <vector>

#include <kernel.hpp>
#include <kernevt.hpp>
#include <hardwaretask.hpp>

namespace RTSim {

  class HardwareTask;

  class FPGAKernelExc : public BaseExc {
    public:
      FPGAKernelExc(string msg) :BaseExc(msg, "FPGAKernel", "fpgakernel.cpp") {}
  };

  struct Slot {
      AbsRTTask *task;
      Scheduler *scheduler;
      CPU *cpu;
      CPU *old_cpu;
  };

  enum DispatcherAlgorithm {
    DISPATCHER_FIRST,         // Returns the first available partition
    DISPATCHER_LESS_WAITING_T,// Returns the partition with the shortest
                              // number of pending tasks
    DISPATCHER_MORE_FREE_SLOTS// Returns the partition with the higher
                              // number of free slots
  };

  class FPGAKernel : public AbsKernel {

      absCPUFactory *_CPUFactory;
      multimap<Scheduler *, Slot> scheduler;
      unsigned int cpu_index = 0;

      DispatcherAlgorithm disp_alg;

      unsigned int number_of_slots(Scheduler *s);

    public:
      FPGAKernel(DispatcherAlgorithm da);
      ~FPGAKernel();


      Scheduler *addPartition(int slotNum);

      void addTask(AbsRTTask &t, const string &params);

      void activate(AbsRTTask *t);
      void suspend(AbsRTTask *t);
      void dispatch();
      void onArrival(AbsRTTask *t);
      void onEnd(AbsRTTask *t);

      Scheduler *dispatcher(const vector<Scheduler *>&affinity);

      virtual CPU *getProcessor(const AbsRTTask *t) const;
      virtual CPU *getOldProcessor(const AbsRTTask *t) const;

      virtual double getSpeed() const { return 0; }
      double setSpeed(double s) { return 0; }
      bool isContextSwitching() const { return false; }
  };
} // namespace RTSim

#endif
