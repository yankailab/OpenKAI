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
        m_pTros = NULL;

#ifdef WITH_3D
        m_pPCframe = NULL;
#endif
    }

    _ROS_fastLio::~_ROS_fastLio()
    {
    }

    bool _ROS_fastLio::init(void *pKiss)
    {
        IF_F(!this->_NavBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        Kiss *pKr = pK->child("threadROS");
        IF_F(pKr->empty());
        m_pTros = new _Thread();
        if (!m_pTros->init(pKr))
        {
            DEL(m_pTros);
            return false;
        }

        rclcpp::init(0, NULL);
        m_pROSnode = std::make_shared<ROS_fastLio>();
        Kiss *pKn = pK->child("node");
        return m_pROSnode->init(pKn);
    }

    bool _ROS_fastLio::link(void)
    {
        IF_F(!this->_NavBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;

#ifdef WITH_3D
        n = "";
        pK->v("_PCframe", &n);
        m_pPCframe = (_PCframe *)(pK->findModule(n));
        m_pROSnode->m_pPCframe = m_pPCframe;
#endif

        return true;
    }

    bool _ROS_fastLio::start(void)
    {
        NULL_F(m_pT);
        IF_F(!m_pT->start(getUpdate, this));

        NULL_F(m_pTros);
        IF_F(!m_pTros->start(getUpdateROS, this));

        return true;
    }

    int _ROS_fastLio::check(void)
    {
        return this->_NavBase::check();
    }

    void _ROS_fastLio::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPSfrom();

            updateNav();

            m_pT->autoFPSto();
        }
    }

    void _ROS_fastLio::updateNav(void)
    {
        IF_(check() != OK_OK);

        m_mT = m_pROSnode->m_mT;
        m_vT = m_pROSnode->m_vP;
        m_vQ = m_pROSnode->m_vQ;
		m_confidence = 30.0;

    }

    void _ROS_fastLio::updateROS(void)
    {
        m_pROSnode->createSubscriptions();

        rclcpp::spin(m_pROSnode);
        rclcpp::shutdown();
    }

    void _ROS_fastLio::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_NavBase::console(pConsole);

        // _Console *pC = (_Console *)pConsole;
        // m_pROSnode->console(pConsole);
    }

}
