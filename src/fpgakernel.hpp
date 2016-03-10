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

  class FPGAKernel : public AbsKernel {

      absCPUFactory *_CPUFactory;
      multimap<Scheduler *, Slot> scheduler;
      unsigned int cpu_index = 0;

    public:
      FPGAKernel();
      ~FPGAKernel();

      Scheduler *addPartition(int slotNum);

      void addTask(AbsRTTask &t, const string &params);

      void activate(AbsRTTask *t);
      void suspend(AbsRTTask *t);
      void dispatch();
      void onArrival(AbsRTTask *t);
      void onEnd(AbsRTTask *t);

      virtual CPU *getProcessor(const AbsRTTask *t) const {
        DBGENTER(_KERNEL_DBG_LEV);

        multimap<Scheduler *, Slot>::const_iterator it;

        for (it = scheduler.begin(); it!=scheduler.end(); ++it)
          if ((*it).second.task == t)
            return (*it).second.cpu;

        return getOldProcessor(t);
      }
      virtual CPU *getOldProcessor(const AbsRTTask *t) const {

        multimap<Scheduler *, Slot>::const_iterator it;

        for (it = scheduler.begin(); it!=scheduler.end(); ++it)
          if ((*it).second.task == t)
            return (*it).second.old_cpu;

        return nullptr;
      }

      virtual double getSpeed() const { return 0; }
      double setSpeed(double s) { return 0; }
      bool isContextSwitching() const { return false; }
  };
} // namespace RTSim

#endif
