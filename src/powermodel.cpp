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

	// Base Parent Class
	//
    PowerModel::PowerModel(double v, unsigned long int f) 
    {
        _V = v;
        _F = f;
    }

    // Minimal Class
    //
    void PowerModelMinimal::update()
    {
        _P = (_V * _V) * _F;
    }

    // BP Class
    //
    PowerModelBP::PowerModelBP(double v, double f, double g, double e,
                               double a, double c) : PowerModel(v, f)
    {
        setGamma(g);
        setEta(e);
        setAlpha(a);
        setCapacitance(c);
    }

    void PowerModelBP::setGamma(double g)
    {
        _gamma = g;
    }

    void PowerModelBP::setEta(double e)
    {
        _eta = e;
    }

    void PowerModelBP::setAlpha(double a)
    {
        _alpha = a;
    }

    void PowerModelBP::setCapacitance(double c)
    {
        _C = c;
    }

    void PowerModelBP::update()
    {
        // Evaluation of the P_charge
        _P_charge = _alpha * _C * _F * (_V * _V);

        // Evaluation of the P_short
        _P_short =  _eta * _P_charge;

        // Evalution of the P_dyn
        _P_dyn = _P_short + _P_charge;

        // Evaluation of P_leak
        _P_leak = _gamma * _V * _P_dyn;

        // Evaluation of the total Power
        _P = _P_leak + _P_dyn;
    }

} // namespace RTSim
