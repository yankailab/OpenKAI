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
        m_pT = NULL;
    }

    _ModuleBase::~_ModuleBase()
    {
        DEL(m_pT);
    }

    bool _ModuleBase::init(void *pKiss)
    {
        IF_F(!this->BASE::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
        
        Kiss *pKt = pK->child("thread");
        IF_d_T(pKt->empty(), LOG_E("Thread not found"));

        m_pT = new _Thread();
        if (!m_pT->init(pKt))
        {
            DEL(m_pT);
            LOG_E("Thread init failed");
            return false;
        }

        return true;
    }

    bool _ModuleBase::link(void)
    {
        IF_F(!this->BASE::link());
        IF_F(!m_pT->link());

        return true;
    }

    bool _ModuleBase::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _ModuleBase::check(void)
    {
        NULL__(m_pT, -1);

        return BASE::check();
    }

    void _ModuleBase::update(void)
    {
    }

    void _ModuleBase::sleep(void)
    {
        IF_(check() < 0);
        m_pT->sleep();
    }

    void _ModuleBase::run(void)
    {
        IF_(check() < 0);
        m_pT->run();
    }

	void _ModuleBase::onGoSleep(void)
    {
        IF_(!m_pT->bOnGoSleep());
    }

	void _ModuleBase::onWakeUp(void)
    {
        IF_(!m_pT->bOnWakeUp());
    }

    void _ModuleBase::console(void *pConsole)
    {
        this->BASE::console(pConsole);

        NULL_(m_pT);
        m_pT->console(pConsole);
    }

}
