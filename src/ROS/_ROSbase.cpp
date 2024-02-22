/*
 * _ROSbase.cpp
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#include "_ROSbase.h"

namespace kai
{

    _ROSbase::_ROSbase()
    {
        m_topic = "";
    }

    _ROSbase::~_ROSbase()
    {
    }

    bool _ROSbase::init(void *pKiss)
    {
        IF_F(!_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("topic", &m_topic);

        return true;
    }

    bool _ROSbase::link(void)
    {
        IF_F(!this->_ModuleBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        return true;
    }

    bool _ROSbase::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _ROSbase::check(void)
    {
        return _ModuleBase::check();
    }

    void _ROSbase::update(void)
    {
        rclcpp::init(0, NULL);

        m_pROSnode = std::make_shared<ROSnode>();
        m_pROSnode->createSubscriptions();

        rclcpp::spin(m_pROSnode);
        rclcpp::shutdown();
    }

    void _ROSbase::updateROS(void)
    {
        IF_(check() < 0);
    }

    void _ROSbase::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        //		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);
    }

}
