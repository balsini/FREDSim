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
/*
 * $Id: resource.cpp,v 1.4 2005/04/28 01:34:48 cesare Exp $
 *
 * $Log: resource.cpp,v $
 * Revision 1.4  2005/04/28 01:34:48  cesare
 * Moved to sstream. Headers install. Code cleaning.
 *
 * Revision 1.3  2004/11/26 03:47:10  cesare
 * Finished merging the main trunk with Lipari's branch.
 *
 */
#include <abstask.hpp>
#include <resource.hpp>

namespace RTSim {

    using namespace std;
    using namespace MetaSim;

    Resource::Resource(const string &n, int nr) :
        Entity(n),
        _owner(nullptr),
        _total(nr),
        _available(nr)
    { 
    }

    Resource::Resource(const Resource &r) :
        Entity(r.getName()+"_copy"), _owner(nullptr),
        _total(r.total()),
        _available(r.total())
    { 
    }

    void Resource::lock(AbsRTTask *t, int n)
    { 
        _available -= n;
        _owner = t;
    }

    void Resource::unlock(int n)
    { 
        _available += n;
        _owner = nullptr;
    }

    bool Resource::isLocked() const
    { 
        return (_available == 0);
    }     

    int  Resource::available() const
    { 
        return _available;
    }

    int  Resource::total() const
    { 
        return _total;
    }

    void Resource::newRun()
    {
      _owner = nullptr;
      _available = _total;
    }

    void Resource::endRun()
    {
    }

//     void Resource::setOwner(AbsRTTask* t)
//     {
//         owner = t;
//     }

    AbsRTTask* Resource::getOwner()const
    {
        return _owner;
    }

}
