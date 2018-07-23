/***************************************************************************
    begin                : Thu Apr 24 15:54:58 CEST 2003
    copyright            : (C) 2003 by Giuseppe Lipari
    email                : lipari@sssup.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __CPU_HPP__
#define __CPU_HPP__

#include <set>
#include <string>
#include <vector>

#include <trace.hpp>

#include <timer.hpp>

#include "powermodel.hpp"

#define _KERNEL_DBG_LEV "Kernel"

namespace RTSim
{

    using namespace std;
    using namespace MetaSim;

    struct OPP
    {
        /// Voltage of each step (in Volts)
        double voltage;

        /// Frequency of each step (in MHz)
        unsigned int frequency;

        /// The speed is a value between 0 and 1
        double speed;
    };

    /**
     * \ingroup kernels
     *
     * A CPU doesn't know anything about who's running on it: it just has a
     * speed factor. This model contains the energy values (i.e. Voltage and
     * Frequency) of each step. The speed of each step is calculated basing
     * upon the step frequencies.  The function setSpeed(load) adjusts the CPU
     * speed accordingly to the system load, and returns the new CPU speed.
     */
    class CPU : public Entity
    {

            /**
             *  Energy model of the CPU
             */
            PowerModel *powmod;
            
            /**
             * Base workload
             */
            double K_base;
            
            /**
             * Delta workload
             */
            double Kw;

            vector<OPP> OPPs;

            /// Name of the CPU
            string cpuName;

            /// currentOPP is a value between 0 and OPPs.size() - 1
            unsigned int currentOPP;

            bool PowerSaving;

            /// Number of speed changes
            unsigned long int frequencySwitching;

            // this is the CPU index in a multiprocessor environment
            int index;

        public:
            /// Constructor for CPUs
            CPU(const string &name="",
                const vector<double> &V= {},
                const vector<unsigned int> &F= {},
                PowerModel *pm = nullptr, double K0 = 0);

            ~CPU();

            /// set the processor index
            void setIndex(int i)
            {
                index = i;
            }

            /// get the processor index
            int getIndex()
            {
                return index;
            }

            /// Useful for debug
            virtual int getCurrentOPP();

            /// Returns the maximum power consumption obtainable with this
            /// CPU
            virtual double getMaxPowerConsumption();

            /// Returns the current power consumption of the CPU If you
            /// need a normalized value between 0 and 1, you should divide
            /// this value using the getMaxPowerConsumption() function.

            virtual double getCurrentPowerConsumption();

            /// Returns the current power saving of the CPU
            virtual double getCurrentPowerSaving();

            /** Sets a new speed for the CPU accordingly to the system
             *  load.  Returns the new speed.
             */
            virtual double setSpeed(double newLoad);
            
            /**
             * Set the computation workload on the cpu
             */
            virtual void setWorkload(double Kw);

            /// Returns the current CPU speed (between 0 and 1)
            virtual double getSpeed();

            virtual double getSpeed(unsigned int OPP);

            virtual unsigned long int getFrequencySwitching();

            virtual void newRun() {}
            virtual void endRun() {}

            ///Useful for debug
            virtual void check();
    };


    /**
     * The abstract CPU factory. Is the base class for every CPU factory which
     * will be implemented.
     */
    class absCPUFactory
    {
        public:
            virtual CPU* createCPU(const string &name="",
                const vector<double> &V= {},
                const vector<unsigned int> &F= {},
                PowerModel *pm = nullptr, double K0 = 0) = 0;
            virtual ~absCPUFactory() {}
    };


    /**
     * uniformCPUFactory. A factory of uniform CPUs (whose speeds are maximum).
     * Allocates a CPU and returns a pointer to it
     */
    class uniformCPUFactory : public absCPUFactory
    {
            char** _names;
            int _curr;
            int _n;
            int index;
        public:
            uniformCPUFactory();
            uniformCPUFactory(char* names[], int n);
            /*
             * Allocates a CPU and returns a pointer to it
             */
            CPU* createCPU(const string &name="",
                const vector<double> &V= {},
                const vector<unsigned int> &F= {},
                PowerModel *pm = nullptr, double K0 = 0);
    };

    /**
     * Stores already created CPUs and returns the pointers, one by one, to the
     * requesting class.
     */
    class customCPUFactory : public absCPUFactory
    {

            list<CPU *> CPUs;

        public:

            customCPUFactory() {}

            void addCPU(CPU *c)
            {
                CPUs.push_back(c);
            }

            /*
             * Returns the pointer to one of the stored pre-allocated CPUs.
             */
            CPU *createCPU(const string &name="",
                const vector<double> &V= {},
                const vector<unsigned int> &F= {},
                PowerModel *pm = nullptr, double K0 = 0)
            {
                if (CPUs.size() > 0)
                {
                    CPU *ret = CPUs.front();
                    CPUs.pop_front();
                    return ret;
                }
                return nullptr;
            }
    };

} // namespace RTSim

#endif
