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

        DEL(m_pT);
        m_pT = createThread(jK(j, "thread"), "thread");
        NULL_F(m_pT);

        return true;
    }

    _Thread *_ModuleBase::createThread(const json &j, const string &name)
    {
        IF_N(name.empty());
        IF_Le__(!j.is_object(), "JSON is not an object: " + name, nullptr);

        _Thread *pT = new _Thread();
        NULL_N(pT);
        pT->setName(name);

        if (!pT->init(j))
        {
            DEL(pT);
            LOG_E("thread.init() failed: " + name);
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

    bool _ModuleBase::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->BASE::link(j, pM));
        IF_F(!m_pT->link(jK(j, "thread"), pM));

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
