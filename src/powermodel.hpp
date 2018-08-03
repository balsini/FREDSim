/***************************************************************************
begin                : Thu Jul 14:23:58 CEST 2018
copyright            : (C) 2018 by Luigi Pannocchi
email                : l.pannocchi@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __POWERMODEL_HPP__
#define __POWERMODEL_HPP__

#include <cstdlib>
#include <cmath>
#include <map>
#include <string>

#define _KERNEL_DBG_LEV "Kernel"

namespace RTSim
{

    using namespace std;
    class CPU;

    class CPUModel {

        CPU *_cpu;

    protected:

        // Outputs
        /**
             * Total power consumption in Watt
             */
        double _P;

        // Inputs

        /**
             * Voltage of the processor
             */
        double _V;

        /**
             * Frequency of the processor
             */
        unsigned long int _F;

        /**
             * Maximum frequency of the processor
             */
        unsigned long int _F_max;

    public:
        /**
             * Default Constructor
             */
        CPUModel(double v = 0, unsigned long int f = 0);

        void setCPU(CPU *c);

        CPU *getCPU();

        // ----------------------
        // Power
        // ----------------------
        /*!
             * Get the instantaneous power consumption
             */
        virtual double getPower() const;

        virtual long double getSpeed();

        /*!
             * Update the power consumption
             */
        virtual void update() = 0;

        // ----------------------
        // Inputs
        // ----------------------
        /*!
             * Set Voltage
             * \param f Voltage in V
             */
        void setVoltage(double v);

        /*!
             * Set Frequency\
             * \param f Frequency in MHz
             */
        void setFrequency(unsigned long int f);

        /*!
         * \brief setFrequencyMax Defines the maximum frequency among all the
         * cores of the system (used to scale the computing time of a task).
         * \param f
         */
        void setFrequencyMax(unsigned long int f);

};

    class CPUModelMinimal : public CPUModel {
    public:
        CPUModelMinimal(double v, unsigned long int f);

        // ----------------------
        // Power
        // ----------------------

        /*!
             * Update the power consumption
             */
        void update();

    };

    class CPUModelBP : public CPUModel {

    public:
        // =============================================
        // Parameters of the energy model
        // =============================================

        struct PowerModelBPParams {
            /**
             * Constant "displacement"
             * TODO
             */
            double d;
            /**
             * Constant "eta"
             * Factor modeling the P_short ( P_short = eta * P_charge)
             */
            double e;
            /**
             * Constant "gamma"
             * Factor modeling the Temperature effect on
             * P_leak (P_leak = gamma * V * P_dyn)
             */
            double g;
            /**
             * Constant "K"
             * Factor modeling the percentage
             * of CPU activity
             */
            double k;
        };

        struct ComputationalModelBPParams {
            long double a;
            long double b;
            long double c;
            long double d;
        };

    private:

        // ==============================
        // Power Variables
        // ==============================

        map<string, PowerModelBPParams> _wl_param;
        map<string, ComputationalModelBPParams> _comp_param;
        /**
             * Variable P_leak
             * Power consumption due to leakage
             * effects
             */
        double _P_leak;

        /**
             * Variable P_dyn
             * Power consumption due to the
             * transistors switching
             */
        double _P_dyn;

        /**
             * Variable P_short
             * Part of the dynamic power consumption due to
             * short circuit effect during the switching
             */
        double _P_short;

        /**
             * Variable P_charge
             * Part of the dynamic power consumption due to
             * the charging of the gate capacitors
             */
        double _P_charge;

        long double speedModel(const ComputationalModelBPParams &m,
                          unsigned long int f) const;

    public:

        CPUModelBP(double v, unsigned long f,
                     double g_idle = 0,
                     double e_idle = 0,
                     double k_idle = 0,
                     double d_idle = 0);

        void setWorkloadParams(const string &workload_name,
                               const PowerModelBPParams &power_params,
                               const ComputationalModelBPParams &computing_params);

        long double getSpeed();

        void update();
    };

} // namespace RTSim

#endif

