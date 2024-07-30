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
        m_pTros = nullptr;

#ifdef WITH_3D
        m_pPCframe = nullptr;
#endif
    }

    _ROS_fastLio::~_ROS_fastLio()
    {
    }

    int _ROS_fastLio::init(void *pKiss)
    {
        CHECK_(this->_NavBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        Kiss *pKr = pK->child("threadROS");
        if (pKr->empty())
        {
            LOG_E("threadROS not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTros = new _Thread();
        CHECK_d_l_(m_pTros->init(pKr), DEL(m_pTros), "threadROS init failed");

        rclcpp::init(0, NULL);
        m_pROSnode = std::make_shared<ROS_fastLio>();
        Kiss *pKn = pK->child("node");
        return m_pROSnode->init(pKn);
    }

    int _ROS_fastLio::link(void)
    {
        CHECK_(this->_NavBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;
#ifdef WITH_3D
        n = "";
        pK->v("_PCframe", &n);
        m_pPCframe = (_PCframe *)(pK->findModule(n));
        m_pROSnode->m_pPCframe = m_pPCframe;
#endif

        return OK_OK;
    }

    int _ROS_fastLio::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        CHECK_(m_pT->start(getUpdate, this));

        NULL__(m_pTros, OK_ERR_NULLPTR);
        CHECK_(m_pTros->start(getUpdateROS, this));

        return OK_OK;
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
