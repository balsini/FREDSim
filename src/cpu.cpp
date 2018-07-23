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

#include <cpu.hpp>

namespace RTSim
{

    CPU::CPU(const string &name,
             const vector<double> &V,
             const vector<unsigned int> &F,
             PowerModel *pm) :

        Entity(name), frequencySwitching(0), index(0)
    {
        auto num_OPPs = V.size();

        cpuName = name;

        if (num_OPPs == 0)
        {
            PowerSaving = false;
            return;
        }

        PowerSaving = true;

        // Setting voltages and frequencies
        for (int i = 0; i < num_OPPs; i ++)
        {
            OPP opp;
            opp.voltage = V[i];
            opp.frequency = F[i];
            OPPs.push_back(opp);
        }

        // Setting speeds (basing upon frequencies)
        for (vector<OPP>::iterator iter = OPPs.begin();
                iter != OPPs.end(); iter++)
            (*iter).speed = ((double)(*iter).frequency) /
                            ((double)F[num_OPPs -1]);

        /* Use the maximum OPP by default */
        currentOPP = num_OPPs - 1;

        // Creating the Energy Model class
        // and initialize it with the max values
        if (!pm)
        {
            powmod = new PowerModelMinimal(OPPs[currentOPP].voltage,
                                           OPPs[currentOPP].frequency);
        }
        else
        {
            powmod = pm;
        }

    }

    CPU::~CPU()
    {
        OPPs.clear();
        delete powmod; // Destroy the PowerModel class
    }

    int CPU::getCurrentOPP()
    {
        if (PowerSaving)
            return currentOPP;
        return 0;
    }

    double CPU::getMaxPowerConsumption()
    {
        if (PowerSaving)
        {
            int opp = OPPs.size() - 1;

            powmod->setVoltage(OPPs[opp].voltage);
            powmod->setFrequency(OPPs[opp].frequency);
            powmod->update();

            return (powmod->getPower());
        }
        return 0;
    }

    double CPU::getCurrentPowerConsumption()
    {
        if (PowerSaving)
        {
            powmod->setVoltage(OPPs[currentOPP].voltage);
            powmod->setFrequency(OPPs[currentOPP].frequency);
            powmod->update();

            return (powmod->getPower());
        }
        return 0;
    }

    double CPU::getCurrentPowerSaving()
    {
        if (PowerSaving)
        {
            long double maxPowerConsumption = getMaxPowerConsumption();
            long double saved = maxPowerConsumption - getCurrentPowerConsumption();
            return static_cast<double>(saved / maxPowerConsumption);
        }
        return 0;
    }

    double CPU::setSpeed(double newLoad)
    {
        DBGENTER(_KERNEL_DBG_LEV);
        DBGPRINT("pwr: setting speed in CPU::setSpeed()");
        DBGPRINT("pwr: New load is " << newLoad);
        if (PowerSaving)
        {
            DBGPRINT("pwr: PowerSaving=on");
            DBGPRINT("pwr: currentOPP=" << currentOPP);
            for (int i=0; i < (int) OPPs.size(); i++)
                if (OPPs[i].speed >= newLoad)
                {
                    if (i != currentOPP)
                        frequencySwitching++;
                    currentOPP = i;
                    DBGPRINT("pwr: New OPP=" << currentOPP <<" New Speed=" << OPPs[currentOPP].speed);

                    return OPPs[i].speed; //It returns the new speed
                }
        }
        else
            DBGPRINT("pwr: PowerSaving=off => Can't set a new speed!");

        return 1; // An error occurred or PowerSaving is not enabled
    }

    double CPU::getSpeed()
    {
        if (PowerSaving)
            return OPPs[currentOPP].speed;
        return 1;
    }

    double CPU::getSpeed(unsigned int OPP)
    {
        if ((!PowerSaving) || ((OPP + 1) > OPPs.size()))
            return 1;
        return OPPs[OPP].speed;
    }

    unsigned long int CPU::getFrequencySwitching()
    {
        DBGENTER(_KERNEL_DBG_LEV);
        DBGPRINT("frequencySwitching=" << frequencySwitching);

        return frequencySwitching;
    }

    void CPU::check()
    {
        cout << "Checking CPU:" << cpuName << endl;;
        cout << "Max Power Consumption is :" << getMaxPowerConsumption() << endl;
        for (vector<OPP>::iterator iter = OPPs.begin(); iter != OPPs.end(); iter++)
        {
            cout << "-OPP-" << endl;
            cout << "\tFrequency:" << (*iter).frequency << endl;
            cout << "\tVoltage:" << (*iter).voltage << endl;
            cout << "\tSpeed:" << (*iter).speed << endl;
        }
        for (unsigned int i = 0; i < OPPs.size(); i++)
            cout << "Speed level" << getSpeed(i) << endl;
        for (vector<OPP>::iterator iter = OPPs.begin(); iter != OPPs.end(); iter++)
        {
            cout << "Setting speed to " << (*iter).speed << endl;
            setSpeed((*iter).speed);
            cout << "New speed is  " << getSpeed() << endl;
            cout << "Current OPP is  " << getCurrentOPP() << endl;
            cout << "Current Power Consumption is  " << getCurrentPowerConsumption() << endl;
            cout << "Current Power Saving is  " << getCurrentPowerSaving() << endl;
        }
    }





    uniformCPUFactory::uniformCPUFactory()
    {
        _curr=0;
        _n=0;
        index = 0;
    }

    uniformCPUFactory::uniformCPUFactory(char* names[], int n)
    {
        _n=n;
        _names = new char*[n];
        for (int i=0; i<n; i++)
        {
            _names[i]=names[i];
        }
        _curr=0;
        index = 0;
    }

    CPU* uniformCPUFactory::createCPU(const string &name,
                                      const vector<double> &V,
                                      const vector<unsigned int> &F)
    {
        CPU *c;

        if (_curr==_n)
            c = new CPU(name, V, F);
        else
            c = new CPU(_names[_curr++], V, F);

        c->setIndex(index++);
        return c;
    }
}
