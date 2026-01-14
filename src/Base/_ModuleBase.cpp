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
        m_pT = nullptr;
        pthread_mutex_init(&m_mutexAtomic, NULL);
    }

    _ModuleBase::~_ModuleBase()
    {
        DEL(m_pT);
        pthread_mutex_destroy(&m_mutexAtomic);
    }

    bool _ModuleBase::init(const json &j)
    {
        IF_F(!this->BASE::init(j));

        IF_Le_F(!j.contains("thread"), "json: thread not found");
        DEL(m_pT);
        m_pT = new _Thread();
        m_pT->setName("thread");
        if (!m_pT->init(j.at("thread")))
        {
            DEL(m_pT);
            LOG_E("thread.init() failed");
            return false;
        }

        return true;
    }

    bool _ModuleBase::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->BASE::link(j, pM));
        IF_F(!m_pT->link(j.at("thread"), pM));

        return true;
    }

    bool _ModuleBase::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    bool _ModuleBase::check(void)
    {
        NULL_F(m_pT);

        return BASE::check();
    }

    void _ModuleBase::update(void)
    {
    }

    bool _ModuleBase::bAlive(void)
    {
        IF_F(!check());

        return m_pT->bAlive();
    }

    bool _ModuleBase::bRun(void)
    {
        IF_F(!check());

        return m_pT->bRun();
    }

    bool _ModuleBase::bStop(void)
    {
        IF_F(!check());

        return m_pT->bStop();
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

    void _ModuleBase::atomicFrom(void)
    {
        pthread_mutex_lock(&m_mutexAtomic);
    }

    void _ModuleBase::atomicTo(void)
    {
        pthread_mutex_unlock(&m_mutexAtomic);
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
