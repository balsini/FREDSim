#ifndef __CONFIGUREINSTR_HPP__
#define __CONFIGUREINSTR_HPP__

#include <memory>

//From metasim
#include <debugstream.hpp>
#include <gevent.hpp>
#include <randomvar.hpp>
#include <simul.hpp>

//From RTLIB
#include <exeinstr.hpp>

namespace RTSim {

  using namespace std;
  using namespace MetaSim;

  /**
     \ingroup instr

     @author Alessio Balsini
     @version 2.0
     @see Instr, Task
  */
  class ConfigureInstr : public FixedInstr {
  public:

    ConfigureInstr(Task *t, Tick duration, const string &n = "") :
      FixedInstr(t, duration, n)
    {}

    virtual void schedule() throw (InstrExc);
    virtual void deschedule();
    virtual void onEnd();

    static Instr *createInstance(vector<string> &par);
  };

} // namespace RTSim

#endif
