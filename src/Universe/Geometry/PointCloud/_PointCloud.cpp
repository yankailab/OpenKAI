/*
 * _PointCloud.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PointCloud.h"

namespace kai
{
    _PointCloud::_PointCloud()
    {
    }

    _PointCloud::~_PointCloud()
    {
    }

    bool _PointCloud::link(InstanceMgr *pM)
    {
        if (!_GeometryBase::link(pM))
        {
            return false;
        }

        string name;
        jKv(*m_pJ, "PCLframe", name);
        m_pPCL = dynamic_cast<PCLframe *>(static_cast<DataStreamBase *>(pM->findDataStream(name)));
        IF_Le_F(!m_pPCL, "PCLframe not found: " + name);
        return true;
    }

    bool _PointCloud::check(void)
    {
        return m_pPCL && _GeometryBase::check();
    }

    void _PointCloud::clear(void)
    {
        if (m_pPCL)
        {
            m_pPCL->set({});
        }
    }
}
