/***************************************************************************
    begin                : Thu Jul 21 15:54:58 CEST 2018
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
#include <powermodel.hpp>

namespace RTSim {

    // Constructors/Destructor

    PowerModel::PowerModel(double v, unsigned long int f) 
    { 
        _V = v;
        _F = f;
    }

    // Methods
    void PowerModelMinimal::update() {
        _P = ( _V * _V ) * _F;
    }
    
} // namespace RTSim
