/*
 * _GeometryBase.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_GeometryBase.h"

namespace kai
{

    _GeometryBase::_GeometryBase()
    {
        m_type = geometry_unknown;
    }

    _GeometryBase::~_GeometryBase()
    {
    }

    bool _GeometryBase::init(const json &j)
    {
        IF_F(!this->_ModuleBase::init(j));

        return true;
    }

    bool _GeometryBase::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_ModuleBase::link(j, pM));

        return true;
    }

    bool _GeometryBase::check(void)
    {
        return this->_ModuleBase::check();
    }

    GEOMETRY_TYPE _GeometryBase::getType(void)
    {
        return m_type;
    }

    void _GeometryBase::clear(void)
    {
    }

    int _GeometryBase::get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pGrPout, uint64_t dTexpire)
    {
        return 0;
    }

    int _GeometryBase::get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pGrLOut, uint64_t dTexpire)
    {
        return 0;
    }

    void _GeometryBase::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
    }

}
