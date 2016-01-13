#include "acceleratedtask.hpp"

namespace RTSim {
    AcceleratedTask::AcceleratedTask(Tick iat)
        : Task(new DeltaVar(iat), iat, 0, "", 1000), period(iat)
    {
    }

    AcceleratedTask::AcceleratedTask(Tick iat, Tick rdl, Tick ph,
			       const std::string &name, long qs)
	: Task(new DeltaVar(iat), rdl, ph, name, qs), period(iat) 
    {
    }

    AcceleratedTask* AcceleratedTask::createInstance(vector<string>& par)
    {
	Tick i = Tick(par[0]);
	Tick d = Tick(par[1]);
	Tick p = Tick(par[2]);
    const char* n = "";
	if (par.size() > 2) n = par[3].c_str();
	long q = 100;
	if (par.size() > 4) q = atoi(par[4].c_str());
	
	// @todo what is a? 
	bool a = true;
	if (par.size() > 5 && !strcmp(par[5].c_str(), "false")) a = false;
        return new AcceleratedTask(i, d, p, n, q);
    }
}
