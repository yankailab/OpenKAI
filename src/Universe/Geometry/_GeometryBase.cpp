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

    bool _GeometryBase::saveConfig(bool bExport)
    {
        IF_F(!_ReferenceFrame::saveConfig(false));

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

    bool _GeometryBase::link(InstanceMgr *pM)
    {
        IF_F(!this->_ReferenceFrame::link(pM));

        return true;
    }

    bool _GeometryBase::check(void)
    {
        return this->_ReferenceFrame::check();
    }

    void _GeometryBase::clear(void)
    {
    }

    void _GeometryBase::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ReferenceFrame::console(pConsole);

    }

}
