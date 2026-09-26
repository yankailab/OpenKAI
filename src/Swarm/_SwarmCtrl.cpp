#include "_SwarmCtrl.h"

namespace kai
{

    _SwarmCtrl::_SwarmCtrl()
    {
    }

    _SwarmCtrl::~_SwarmCtrl()
    {
    }

    bool _SwarmCtrl::loadConfig(void)
    {
        IF_F(!this->_ModuleBase::loadConfig());

//        jKv(j, "myID", m_node.m_id);

        return true;
    }

    bool _SwarmCtrl::saveConfig(bool bExport)
    {
        IF_F(!_ModuleBase::saveConfig(false));

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

    bool _SwarmCtrl::link(void)
    {
        IF_F(!this->_ModuleBase::link());

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
