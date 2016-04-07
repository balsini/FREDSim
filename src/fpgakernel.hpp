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
      bool in_use;
      Tick last_used;
  };

  enum DispatcherAlgorithm {
    DISPATCHER_FIRST,         // Returns the first available partition
    DISPATCHER_LESS_WAITING_T,// Returns the partition with the shortest
                              // number of pending tasks
    DISPATCHER_MORE_FREE_SLOTS// Returns the partition with the higher
                              // number of free slots
  };

  enum FRIAlgorithm {
    FP_PREEMPTIVE,
    FP_NONPREEMPTIVE,
    TB_PREEMPTIVE,
    TB_NONPREEMPTIVE
  };

  class FPGAKernel : public Entity, public virtual AbsKernel {

      absCPUFactory *_CPUFactory;
      multimap<Scheduler *, Slot> scheduler;
      unsigned int cpu_index;
      bool fri_locked;

      AbsRTTask * task_locking_FRI;
      vector<AbsRTTask *> FRI_locked_tasks;

      DispatcherAlgorithm disp_alg;
      FRIAlgorithm fri_alg;

      unsigned int number_of_slots(Scheduler *s);

      /// The resource manager
      ResManager* _resMng;

    public:
      FPGAKernel(DispatcherAlgorithm da, FRIAlgorithm fa, const std::string& name = "");
      ~FPGAKernel();


      Scheduler *addPartition(int slotNum);

      void addTask(AbsRTTask &t, const string &params);

      /**
       * This function is called when a spinlock is marked
       * as free
       */
      void activate(AbsRTTask *t);

      /**
       * This function is called when a spinlock is busy,
       * so the task does not interrupt its execution
       */
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
      void FRILock(AbsRTTask * t);
      void FRIUnlock(AbsRTTask * t);

      /**
         Forwards the request of resource r from task t to
         the resource manager. If the resource manager has
         not been set, a RTKernelExc exception is raised.
      */
      virtual bool requestResource(AbsRTTask *t, const string &r, int n=1)
          throw(FPGAKernelExc);

      /**
         Forwards the release of the resource r by task t to
         the resource manager. If the resource manager has
         not been set, a RTKernelExc is raised.
      */
      virtual void releaseResource(AbsRTTask *t, const string &r, int n=1)
          throw(FPGAKernelExc);

      virtual void newRun() {}
      virtual void endRun();

  };
} // namespace RTSim

#endif
