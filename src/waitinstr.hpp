#ifndef __WAITINSTR_HPP__
#define __WAITINSTR_HPP__

#include <string>
#include <vector>

//From metasim
#include <event.hpp>
#include <factory.hpp>

//From RTLIB
#include <instr.hpp>
#include <taskevt.hpp>

namespace RTSim {
  using namespace std;
  using namespace MetaSim;

  class Task;
  class WaitInstr;
  class SignalInstr;

  /**
     \ingroup instr

     event for wait instr
   */
  class WaitEvt : public TaskEvt
  {
  protected:
    WaitInstr * wi;
  public:
    WaitEvt(Task* t, WaitInstr* in) :TaskEvt(t, _DEFAULT_PRIORITY - 3), wi(in)
    {}
    WaitInstr *getInstr() {return wi;} 
    virtual void doit() {}
  };

  /**
     \ingroup instr

     event for signal instr
   */
  class SignalEvt : public TaskEvt
  {
  protected:
    SignalInstr *si;
  public:
    SignalEvt(Task* t, SignalInstr* in) :TaskEvt(t), si(in) {} 
    virtual void doit() {}
    SignalInstr *getInstr() {return si;}
  };

  /** 
      \ingroup instr

      Simple classes which model wait and signal instruction to use a resource 
      @author Fabio Rossi and Giuseppe Lipari
      @see Instr 
   */

  class WaitInstr : public Instr {
    string _res;
    EndInstrEvt _endEvt; 
    WaitEvt _waitEvt;
    int _numberOfRes;
  public:
    /**
       This is the constructor of the WaitInstr.
       @param f is a pointer to the task containing the pseudo
       instruction
       @param r is the name of the resorce manager handling the
       resource which the task is accessing
       @param nr is the number of resources being taken
       @param n is the instruction name
     */
    WaitInstr(Task * f, const char *r, int nr=1, const string &n = "");
    WaitInstr(Task * f, const string &r, int nr=1, const string & n = "");

    static Instr* createInstance(vector<string> &par);

    ///Virtual methods from Instr
    virtual void schedule();
    virtual void deschedule();
    virtual Tick getExecTime() const { return 0;};
    virtual Tick getDuration() const { return 0;};
    virtual Tick getWCET() const throw(RandomVar::MaxException) { return 0; }
    string getResource() const { return _res; };
    virtual void reset() {}
    virtual void setTrace(Trace *);

    virtual void onEnd();
    virtual void newRun() {};
    virtual void endRun();


    /** Function inherited from clss Instr.It refreshes the state 
     *  of the executing instruction when a change of the CPU speed occurs. 
     */ 
    virtual void refreshExec(double, double){}

  };

  /**
     \ingroup instr

     Simple class which models signal instruction to use a resource. 
     @author Fabio Rossi and Giuseppe Lipari
     @see Instr 
   */

  class SignalInstr : public Instr {
    string _res;
    EndInstrEvt _endEvt;
    SignalEvt _signalEvt;
 
    int _numberOfRes;
  public:
    /**
       This is the constructor of the SignalInstr
       @param f is a pointer to the task containing the pseudo
       instruction
       @param r is the name of the resource which the task has
       accessed
       @param nr is the number of resources being taken
       @param n is the instruction name
     */
    SignalInstr(Task *f, const char *r, int nr=1, const string &n = "");
    SignalInstr(Task *f, const string &r, int nr=1, const string &n = "");

    static Instr* createInstance(vector<string> &par);

    ///Virtual methods from Instr
    virtual void schedule();
    virtual void deschedule();
    virtual Tick getExecTime() const { return 0;};
    virtual Tick getDuration() const { return 0;};
    virtual Tick getWCET() const throw(RandomVar::MaxException) {return 0;}
    virtual void reset() {}
    virtual void setTrace(Trace *);
    string getResource() const { return _res; }; 
    virtual void onEnd();
    virtual void newRun() {};
    virtual void endRun();


    /** Function inherited from clss Instr.It refreshes the state 
     *  of the executing instruction when a change of the CPU speed occurs. 
     */ 
    virtual void refreshExec(double, double) {}

  };

} //namespace RTSim

#endif
