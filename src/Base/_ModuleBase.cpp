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
        IF_F(this->BASE::init(j));

        if (!j.contains("thread"))
        {
            LOG_E("json: thread not found");
            return false;
        }

        const json &jt = j.at("thread");
        if (!jt.is_object())
        {
            LOG_E("json: thread is not an object");
            return false;
        }

        DEL(m_pT);
        m_pT = new _Thread();
        if (!m_pT->init(jt))
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

        const json &jt = j.at("thread");
        if (!jt.is_object())
        {
            LOG_E("json: thread is not an object");
            return false;
        }

        IF_F(!m_pT->link(jt, pM));

        return true;
    }

    int _ModuleBase::init(void *pKiss)
    {
        CHECK_(this->BASE::init(pKiss));

        Kiss *pK = (Kiss *)pKiss;
        Kiss *pKt = pK->child("thread");
        if (pKt->empty())
        {
            LOG_E("thread not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pT = new _Thread();
        CHECK_d_l_(m_pT->init(pKt), DEL(m_pT), "thread init failed");

        return OK_OK;
    }

    int _ModuleBase::link(void)
    {
        CHECK_(this->BASE::link());
        CHECK_(m_pT->link());

        return OK_OK;
    }

    int _ModuleBase::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        return m_pT->start(getUpdate, this);
    }

    int _ModuleBase::check(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);

        return BASE::check();
    }

    void _ModuleBase::update(void)
    {
    }

    bool _ModuleBase::bAlive(void)
    {
        IF_F(check() != OK_OK);

        return m_pT->bAlive();
    }

    bool _ModuleBase::bRun(void)
    {
        IF_F(check() != OK_OK);

        return m_pT->bRun();
    }

    bool _ModuleBase::bStop(void)
    {
        IF_F(check() != OK_OK);

        return m_pT->bStop();
    }

    void _ModuleBase::pause(void)
    {
        IF_(check() != OK_OK);

        m_pT->pause();
    }

    void _ModuleBase::resume(void)
    {
        IF_(check() != OK_OK);

        m_pT->run();
    }

    void _ModuleBase::stop(void)
    {
        IF_(check() != OK_OK);

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
