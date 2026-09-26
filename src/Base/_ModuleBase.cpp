/*
 * _ModuleBase.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: yankai
 */

#include "_ModuleBase.h"

namespace kai
{

    _ModuleBase::_ModuleBase()
    {
    }

    _ModuleBase::~_ModuleBase()
    {
        DEL(m_pT);
    }

    bool _ModuleBase::loadConfig(void)
    {
        IF_F(!this->BASE::loadConfig());
        json &j = *m_pJ;

        DEL(m_pT);
        m_pT = createThread(jK(j, "thread"), "thread");
        NULL_F(m_pT);

        return true;
    }

    bool _ModuleBase::saveConfig(bool bExport)
    {
        if (!BASE::saveConfig(false))
        {
            return false;
        }
        if (m_pT && !m_pT->saveConfig(false))
        {
            return false;
        }

        if (!bExport)
        {
            return true;
        }
        return m_pJcfg->saveToFile();
    }

    _Thread *_ModuleBase::createThread(json *pJ, const string &name)
    {
        if (name.empty() || !m_pM || !m_pJcfg || !m_pJ)
        {
            return nullptr;
        }

        if (!pJ)
        {
            (*m_pJ)[name] = json::object();
            pJ = &(*m_pJ)[name];
        }
        if (!pJ->is_object())
        {
            LOG_E("JSON is not an object: " + name);
            return nullptr;
        }

        _Thread *pT = new _Thread();
        pT->setModuleMgr(m_pM);
        pT->setName(name);
        pT->setConfig(m_pJcfg, pJ);
        if (!pT->loadConfig())
        {
            delete pT;
            LOG_E("thread.loadConfig() failed: " + name);
            return nullptr;
        }

        return pT;
    }

    _Thread *_ModuleBase::getThread(const string &name)
    {
        NULL_N(m_pT);
        IF_N(m_pT->getName() != name);

        return m_pT;
    }

    bool _ModuleBase::link(void)
    {
        IF_F(!this->BASE::link());
        IF_F(!m_pT || !m_pT->link());

        return true;
    }

    bool _ModuleBase::start(void)
    {
        NULL_F(m_pT);
        return m_pT->startThread(getUpdate, this);
    }

    bool _ModuleBase::check(void)
    {
        NULL_F(m_pT);

        return BASE::check();
    }

    void _ModuleBase::update(void)
    {
    }

    bool _ModuleBase::bRun(void)
    {
        IF_F(!check());

        return m_pT->bRun();
    }

    bool _ModuleBase::bRunning(void)
    {
        IF_F(!check());

        return m_pT->bRunning();
    }

    bool _ModuleBase::bStopped(void)
    {
        IF_F(!check());

        return m_pT->bStopped();
    }

    void _ModuleBase::pause(void)
    {
        IF_(!check());

        m_pT->pause();
    }

    void _ModuleBase::resume(void)
    {
        IF_(!check());

        m_pT->run();
    }

    void _ModuleBase::stop(void)
    {
        IF_(!check());

        m_pT->stop();
    }

    void _ModuleBase::onPause(void)
    {
    }

    void _ModuleBase::onResume(void)
    {
    }

    void _ModuleBase::console(void *pConsole)
    {
        this->BASE::console(pConsole);

        NULL_(m_pT);
        m_pT->console(pConsole);
    }

}
