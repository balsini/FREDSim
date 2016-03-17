#ifndef __ACCELERATEINSTR_HPP__
#define __ACCELERATEINSTR_HPP__

#include <memory>

//From metasim
#include <debugstream.hpp>
#include <gevent.hpp>
#include <randomvar.hpp>
#include <simul.hpp>

//From RTLIB
#include <instr.hpp>

namespace RTSim {

  using namespace std;
  using namespace MetaSim;

  class AcceleratedTask;

  /**
      \ingroup instr

      These instruction is used to emulate the acceleration request
      by a task.
      The requester task suspends until the accelerator results are
      produced.

      @author Alessio Balsini
      @version 2.0
      */

  class AccelerateInstr : public Instr {

      Tick computation;

    public:

      MetaSim::GEvent<AccelerateInstr> suspEvt;
      MetaSim::GEvent<AccelerateInstr> resumeEvt;

      /** This is the constructor of the AccelerateInstr
    @param f is a pointer to the task containing the pseudo intruction
    @param c is a random variable containing the duration of the instruction
    @param n is the instruction name
    */
      AccelerateInstr(Task * f, Tick d);
      static Instr *createInstance(vector<string> &par);

      virtual void schedule();
      void onSuspend(MetaSim::Event *evt);
      void onEnd(MetaSim::Event *evt);
      void endRun();

      virtual void deschedule() {};
      virtual Tick getExecTime() const { return 0;};
      virtual Tick getDuration() const { return 0;};
      virtual Tick getWCET() const throw(RandomVar::MaxException) { return 0; }
      virtual void reset() {}
      virtual void setTrace(Trace *) {}
      void newRun() {}

      /** Function inherited from Instr. It refreshes the state of the
     *  executing instruction when a change of the CPU speed occurs.
     */
      virtual void refreshExec(double, double){}
  };
} // namespace RTSim

#endif
