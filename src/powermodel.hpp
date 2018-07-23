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

#include <stdlib.h>

#define _KERNEL_DBG_LEV "Kernel"

namespace RTSim
{

    using namespace std;

    class PowerModel
    {

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

        public:
            /**
             * Default Constructor
             */
            PowerModel(double v = 0, unsigned long int f = 0);

            // ----------------------
            // Power
            // ----------------------
            /*!
             * Get the instantaneous power consumption
             */
            double getPower() const
            {
                return _P;
            }

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
            void setVoltage(double v)
            {
                _V = v;
            }

            /*!
             * Set Frequency\
             * \param f Frequency in MHz
             */
            void setFrequency(unsigned long int f)
            {
                _F = 1000000 * f;
            }
            
            /**
             * Set Workload
             * \param Kw Workload
             */
            virtual void setWorkload(double Kw) = 0;
            
    };

    class PowerModelMinimal : public PowerModel
    {
        public:
            PowerModelMinimal(double v, unsigned long int f);

            // ----------------------
            // Power
            // ----------------------

            /*!
             * Update the power consumption
             */
            void update();
    };

    class PowerModelBP : public PowerModel
    {

        private:

            // ==============================
            // Power Variables
            // ==============================

            /**
             * Variable "Kw"
             * Factor modeling the percentage
             * of CPU activity when executing
             */
            double _Kw;

            
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

            // =============================================
            // Parameters of the energy model
            // =============================================
            /**
             * Constant "gamma"
             * Factor modeling the Temperature effect on
             * P_leak (P_leak = gamma * V * P_dyn)
             */
            double _gamma;

            /**
             * Constant "eta"
             * Factor modeling the P_short ( P_short = eta * P_charge)
             */
            double _eta;

            /**
             * Constant "K0"
             * Factor modeling the percentage
             * of CPU activity when Idle
             */
            double _K0;


        public:

            /*!
             * Constructor
             */
            PowerModelBP(double v, double f,
                         double gamma, double eta, double K0);

            /*!
             * Set Frequency\
             * \param wl Workload of the system
             */
            void setWorkload(double Kw);
            
            /*!
            * Set the Gamma factor
            */
            void setGamma(double val);

            /*!
             * Set the Eta factor
             */
            void setEta(double val);

            /*!
             * Set the Initial workload factor
             * \param val Value in [0, inf]
             */
            void setBaseWL(double K0);

            /*!
             * Update the power consumption
             */
            void update();
    };

} // namespace RTSim

#endif

