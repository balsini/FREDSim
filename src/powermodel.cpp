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
#include <cpu.hpp>

namespace RTSim
{

    // Constructors/Destructor

	// Base Parent Class
	//
    PowerModel::PowerModel(double v, unsigned long int f) :
        _cpu(nullptr)
    {
        _V = v;
        _F = f;
    }

    CPU *PowerModel::getCPU()
    {
        return _cpu;
    }

    void PowerModel::setCPU(CPU *c)
    {
        _cpu = c;
    }

    double PowerModel::getPower() const
    {
        return _P;
    }

    long double PowerModel::getSpeed()
    {
        return _F_max / _F;
    }

    void PowerModel::setVoltage(double v)
    {
        _V = v;
    }

    void PowerModel::setFrequency(unsigned long int f)
    {
        _F = 1000 * f;
    }

    void PowerModel::setFrequencyMax(unsigned long int f)
    {
        _F_max = 1000 * f;
    }

    // Minimal Class
    //
    PowerModelMinimal::PowerModelMinimal(double v, unsigned long int f) :
        PowerModel(v, f)
    {
    }

    void PowerModelMinimal::update()
    {
        _P = (_V * _V) * _F;
    }

    // BP Class
    //
    PowerModelBP::PowerModelBP(double v, unsigned long f,
                               double g_idle,
                               double e_idle,
                               double k_idle,
                               double d_idle) :
        PowerModel(v, f)
    {
        PowerModelBPParams mp;

        mp.d = d_idle;
        mp.e = e_idle;
        mp.g = g_idle;
        mp.k = k_idle;

        _wl_param["idle"] = mp;
    }

    void PowerModelBP::update()
    {
        double K, eta, gamma, disp;
        string _curr_wl = getCPU()->getWorkload();

        disp = _wl_param[_curr_wl].d;
        K = _wl_param[_curr_wl].k;
        eta = _wl_param[_curr_wl].e;
        gamma = _wl_param[_curr_wl].g;

        // Evaluation of the P_charge
        _P_charge = (K) * _F * (_V * _V);

        // Evaluation of the P_short
        _P_short =  eta * _P_charge;

        // Evalution of the P_dyn
        _P_dyn = _P_short + _P_charge;

        // Evaluation of P_leak
        _P_leak = gamma * _V * _P_dyn;

        // Evaluation of the total Power
        _P = _P_leak + _P_dyn + disp;

    }

    long double PowerModelBP::speedModel(const ComputationalModelBPParams &m,
                      unsigned long int f) const
    {
        long double disp = m.a;
        long double ideal = m.b / static_cast<long double>(f);
        long double slope = m.c * exp(-(static_cast<long double>(f) / m.d));

        return disp + ideal + slope;
    }

    void PowerModelBP::setWorkloadParams(const string &workload_name,
                                         const PowerModelBPParams &power_params,
                                         const ComputationalModelBPParams &computing_params)
    {
        _wl_param[workload_name] = power_params;
        _comp_param[workload_name] = computing_params;
    }

    long double PowerModelBP::getSpeed()
    {
        string curr_wl = getCPU()->getWorkload();
        long double ret = speedModel(_comp_param[curr_wl], _F);

        return ret;
    }

} // namespace RTSim
