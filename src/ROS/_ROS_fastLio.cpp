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
    }

    _ROS_fastLio::~_ROS_fastLio()
    {
    }

    bool _ROS_fastLio::loadConfig(void)
    {
        IF_F(!this->_NavBase::loadConfig());
        json &j = *m_pJ;

        DEL(m_pTros);
        m_pTros = createThread(jK(j, "threadROS"), "threadROS");
        NULL_F(m_pTros);

        rclcpp::init(0, NULL);
        m_pROSnode = std::make_shared<ROS_fastLio>();
        const json *pJnode = jK(j, "node");
        return pJnode && m_pROSnode->init(*pJnode);
    }

    bool _ROS_fastLio::saveConfig(bool bExport)
    {
        if (!_NavBase::saveConfig(false))
        {
            return false;
        }

        json &j = *m_pJ;
        if (m_pROSnode)
        {
            json &node = j["node"];
            node["topicPC2"] = m_pROSnode->m_topicPC2;
            node["topicOdom"] = m_pROSnode->m_topicOdom;
            node["topicPath"] = m_pROSnode->m_topicPath;
        }

        if (m_pTros && !m_pTros->saveConfig(false))
        {
            return false;
        }

        if (!bExport)
        {
            return true;
        }
        return m_pJcfg->saveToFile();
    }

    bool _ROS_fastLio::link(void)
    {
        IF_F(!this->_NavBase::link());
        const json &j = *m_pJ;
        IF_F(!m_pTros || !m_pTros->link());

        string n;
#ifdef WITH_UNIVERSE
        n = "";
        jKv(j, "_PCframe", n);
        m_pPCframe = (_PCframe *)(m_pM->findModule(n));
        m_pROSnode->m_pPCframe = m_pPCframe;
#endif

        return true;
    }

    bool _ROS_fastLio::start(void)
    {
        NULL_F(m_pT);
        IF_F(!m_pT->startThread(getUpdate, this));

        NULL_F(m_pTros);
        IF_F(!m_pTros->startThread(getUpdateROS, this));

        return true;
    }

    bool _ROS_fastLio::check(void)
    {
        return this->_NavBase::check();
    }

    void _ROS_fastLio::update(void)
    {
        while (m_pT->bRun())
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
