#include <ps_trace.hpp>


namespace RTSim {

    using namespace std;
    using namespace MetaSim;


    PSTrace::PSTrace(const string& name)
    {
        fd.open(name.c_str());
        first_event = true;
    }

    PSTrace::~PSTrace() {
        fd.close();
    }

    void PSTrace::writeTaskEvent(Task &tt, const std::string &evt_name)
    {
      string name = tt.getName();
        fd << SIMUL.getTime() << "\t";
        fd << name << "\t";
        fd << tt.getCPU() << "\t";
        fd << evt_name << "\t";
        fd << endl;
    }

    void PSTrace::probe(ArrEvt& e)
    {
        Task& tt = *(e.getTask());
        writeTaskEvent(tt, "CREATION\tI");
    }

    void PSTrace::probe(EndEvt& e)
    {
        Task& tt = *(e.getTask());
        writeTaskEvent(tt, "RUNNING\tE");
        //writeTaskEvent(tt, "DEAD\tI");
    }

    void PSTrace::probe(SchedEvt& e)
    {
        Task& tt = *(e.getTask());
        writeTaskEvent(tt, "RUNNING\tS");
    }

    void PSTrace::probe(DeschedEvt& e)
    {
        Task& tt = *(e.getTask());
        writeTaskEvent(tt, "RUNNING\tE");
    }

    void PSTrace::probe(DeadEvt& e)
    {
        Task& tt = *(e.getTask());
        writeTaskEvent(tt, "MISS\t\tI");
    }

    void PSTrace::attachToTask(Task* t)
    {
        new Particle<ArrEvt, PSTrace>(&t->arrEvt, this);
        new Particle<EndEvt, PSTrace>(&t->endEvt, this);
        new Particle<SchedEvt, PSTrace>(&t->schedEvt, this);
        new Particle<DeschedEvt, PSTrace>(&t->deschedEvt, this);
        new Particle<DeadEvt, PSTrace>(&t->deadEvt, this);
    }
}
