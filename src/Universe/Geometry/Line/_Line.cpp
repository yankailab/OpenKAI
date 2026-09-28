/*
 * _Line.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_Line.h"

namespace kai
{
    _Line::_Line()
    {
    }

    _Line::~_Line()
    {
    }

    bool _Line::link(InstanceMgr *pM)
    {
        if (!_GeometryBase::link(pM))
        {
            return false;
        }

        string name;
        jKv(*m_pJ, "LineFrame", name);
        m_pLine = dynamic_cast<LineFrame *>(static_cast<DataStreamBase *>(pM->findDataStream(name)));
        IF_Le_F(!m_pLine, "LineFrame not found: " + name);
        return true;
    }

    bool _Line::check(void)
    {
        return m_pLine && _GeometryBase::check();
    }

    void _Line::clear(void)
    {
        if (m_pLine)
        {
            m_pLine->set({});
        }
    }
}
