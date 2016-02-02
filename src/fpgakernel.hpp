#ifndef __FPGAKERNEL_HPP__
#define __FPGAKERNEL_HPP__

#include <vector>

#include <kernel.hpp>
#include <kernevt.hpp>
#include <fifosched.hpp>
#include <hardwaretask.hpp>

namespace RTSim {

  class HardwareTask;
  class FIFOScheduler;

  class FPGAKernel : public AbsKernel {
      vector<Scheduler *> s;
      vector<RTKernel *> k;
      int _areas;
      int _atoms;
    public:
      FPGAKernel(int areas, int atoms) {
        _areas = areas;
        _atoms = atoms;
        for (unsigned int i=0; i<_areas; ++i) {
          s.push_back(new FIFOScheduler());
          k.push_back(new RTKernel(s.back()));
        }
      }
      ~FPGAKernel() {
        for (unsigned int i=0; i<_areas; ++i) {
          delete s.at(i);
          delete k.at(i);
        }
        s.clear();
        k.clear();
      }

      void addTask(AbsRTTask &t, const string &params) {
        HardwareTask * h = dynamic_cast<HardwareTask *>(&t);
        h->setFPGAKernel(this);
        h->setKernel(this);
      }

      void activate(AbsRTTask * t) {
        asm("NOP");
        asm("NOP");
        asm("NOP");
      }
      void suspend(AbsRTTask * t) {
        asm("NOP");
        asm("NOP");
        asm("NOP");
      }
      void dispatch() {
        asm("NOP");
        asm("NOP");
        asm("NOP");
      }
      void onArrival(AbsRTTask * t) {
        unsigned int aff;
        HardwareTask * h;

        h = dynamic_cast<HardwareTask *>(t);
        h->setKernel(0);

        // TODO

        switch (h->getAffinity()) {
          case 1:
            k[0]->addTask(*h, "");
            k[0]->onArrival(h);
            break;
          case 2:
            k[1]->addTask(*h, "");
            k[1]->onArrival(h);
            break;
          default: break;
        }

        //h->setKernel(this);
        //k[h->getAffinity()]->onArrival(t);
      }
      void onEnd(AbsRTTask * t) {
        asm("NOP");
        asm("NOP");
        asm("NOP");
      }

      virtual CPU* getProcessor(const AbsRTTask* t) const { return 0; }
      virtual CPU * getOldProcessor(const AbsRTTask * t) const { return 0; }

      virtual double getSpeed() const { return 0; }
      double setSpeed(double s) { return 0; }
      bool isContextSwitching() const { return false; }
  };
} // namespace RTSim

#endif
