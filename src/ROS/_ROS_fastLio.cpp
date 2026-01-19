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

    bool _ROS_fastLio::init(const json &j)
    {
        IF_F(!this->_NavBase::init(j));

        DEL(m_pTros);
        m_pTros = createThread(j.at("threadROS"), "threadROS");
        NULL_F(m_pTros);

        rclcpp::init(0, NULL);
        m_pROSnode = std::make_shared<ROS_fastLio>();
        return m_pROSnode->init(j.at("node"));
    }

    bool _ROS_fastLio::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_NavBase::link(j, pM));

        string n;
#ifdef WITH_3D
        n = "";
        jKv(j, "_PCframe", n);
        m_pPCframe = (_PCframe *)(pM->findModule(n));
        m_pROSnode->m_pPCframe = m_pPCframe;
#endif

        return true;
    }

    bool _ROS_fastLio::start(void)
    {
        NULL_F(m_pT);
        IF_F(!m_pT->start(getUpdate, this));

        NULL__(m_pTros);
        IF_F(!m_pTros->start(getUpdateROS, this));

        return true;
    }

    bool _ROS_fastLio::check(void)
    {
        return this->_NavBase::check();
    }

    void _ROS_fastLio::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            updateNav();
        }
    }

    void _ROS_fastLio::updateNav(void)
    {
        IF_(!check());

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
