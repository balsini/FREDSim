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

namespace RTSim {

    using namespace std;

    class PowerModel {

    protected:
            // Outputs
            double _P;
            
            // Inputs
            double _V;
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
            double getPower() const { return _P; }

            /*!
             * Update the power consumption
             */
            virtual void update() = 0;

            // ----------------------
            // Inputs
            // ----------------------
            /*!
             * Set Voltage
             */
            void setVoltage(double v) 
            {
                _V = v;
            }

            /*!
             * Set Frequency
             */
            void setFrequency(unsigned long int f) 
            {
                _F = f;
            }
    };

    class PowerModelMinimal : public PowerModel {
        public:
            
            // ----------------------
            // Power
            // ----------------------
            /*!
             * Get the instantaneous power consumption
             */
            double getPower() const;

            /*!
             * Update the power consumption
             */
            void update();
    };
    
} // namespace RTSim
  
#endif

