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
    }

    _ModuleBase::~_ModuleBase()
    {
        DEL(m_pT);
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
        IF_F(check() != OK_OK);;

        return m_pT->bAlive();
    }

    bool _ModuleBase::bRun(void)
    {
        IF_F(check() != OK_OK);;

        return m_pT->bRun();
    }

    bool _ModuleBase::bStop(void)
    {
        IF_F(check() != OK_OK);;

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
