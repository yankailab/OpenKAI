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
    }

    _GeometryBase::~_GeometryBase()
    {
    }

    bool _GeometryBase::loadConfig(void)
    {
        IF_F(!this->_ReferenceFrame::loadConfig());

        return true;
    }

    bool _GeometryBase::link(void)
    {
        IF_F(!this->_ReferenceFrame::link());

        return true;
    }

    bool _GeometryBase::saveConfig(void)
    {
        if (!_ReferenceFrame::saveConfig())
        {
            return false;
        }

        return m_pJcfg->saveToFile();
    }

    bool _GeometryBase::check(void)
    {
        return this->_ReferenceFrame::check();
    }

    GEOMETRY_TYPE _GeometryBase::getType(void)
    {
        return m_type;
    }

    void _GeometryBase::clear(void)
    {
    }

    int _GeometryBase::get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire)
    {
        return 0;
    }

    int _GeometryBase::get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pOut, uint64_t tExpire)
    {
        return 0;
    }

    int _GeometryBase::getLastFrame(vector<Vector3f> *pvP, vector<Vector3f> *pvC, uint64_t &tStamp)
    {
        if (pvP) pvP->clear();
        if (pvC) pvC->clear();
        tStamp = 0;
        return 0;
    }

    void _GeometryBase::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ReferenceFrame::console(pConsole);

//        _Console *pC = (_Console *)pConsole;
    }

}
