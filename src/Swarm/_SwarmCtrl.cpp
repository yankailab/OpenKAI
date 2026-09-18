#include "_SwarmCtrl.h"

namespace kai
{

    _SwarmCtrl::_SwarmCtrl()
    {
    }

    _SwarmCtrl::~_SwarmCtrl()
    {
    }

    bool _SwarmCtrl::init(const json &j)
    {
        IF_F(!this->_ModuleBase::init(j));

//        jKv(j, "myID", m_node.m_id);

        return true;
    }

    bool _SwarmCtrl::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_ModuleBase::link(j, pM));

        return true;
    }

    bool _SwarmCtrl::start(void)
    {
        NULL_F(m_pT);
        return m_pT->startThread(getUpdate, this);
    }

    bool _SwarmCtrl::check(void)
    {
        return this->_ModuleBase::check();
    }

    void _SwarmCtrl::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

        }
    }

    void _SwarmCtrl::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);
    }

}
