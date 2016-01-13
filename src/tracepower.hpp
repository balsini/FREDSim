#ifndef __TRACE_POWER_HPP__
#define __TRACE_POWER_HPP__

#include <cpu.hpp>

namespace RTSim {
    /**
     * \ingroup server
     *
     * This class exports a periodic trace of the power saved by the CPU.
     * The trace is saved on a file called "power.txt" every 10 msec.
     */
    class TracePowerConsumption:
        public PeriodicTimer, public TraceAscii
    {
        CPU* cpu;
        unsigned long long int counter;
        long double totalPowerSaved;
        long double totalPowerConsumed;
    public:
        TracePowerConsumption(CPU* c, Tick period=10, const string &filename="power.txt");
        ~TracePowerConsumption();
        long double getAveragePowerSaving();
        long double getAveragePowerConsumption();
	
        /// Periodically updates the variables and writes some values in the file
        void action();
    };

}

#endif
