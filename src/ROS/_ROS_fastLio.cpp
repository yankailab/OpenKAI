/*
 * _ROS_fastLio.cpp
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#include "_ROS_fastLio.h"

namespace kai
{

    _ROS_fastLio::_ROS_fastLio()
    {

#ifdef WITH_3D
		m_pPCframe = NULL;
#endif

    }

    _ROS_fastLio::~_ROS_fastLio()
    {
    }

    bool _ROS_fastLio::init(void *pKiss)
    {
        IF_F(!this->_ROSbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;


        rclcpp::init(0, NULL);
        m_pROSnode = std::make_shared<ROS_fastLio>();

        Kiss* pKn = pK->child("node");
        return m_pROSnode->init(pKn);
    }

    bool _ROS_fastLio::link(void)
    {
        IF_F(!this->_ROSbase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;

#ifdef WITH_3D
        n = "";
        pK->v("_PCframe", &n);
        m_pPCframe = (_PCframe *)(pK->getInst(n));

        m_pROSnode->m_pPCframe = m_pPCframe;
#endif

        return true;
    }

    bool _ROS_fastLio::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _ROS_fastLio::check(void)
    {
        return this->_ROSbase::check();
    }

    void _ROS_fastLio::update(void)
    {
        m_pROSnode->createSubscriptions();

        rclcpp::spin(m_pROSnode);
        rclcpp::shutdown();
    }

    void _ROS_fastLio::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ROSbase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        //		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);

        m_pROSnode->console(pConsole);

    }

}
