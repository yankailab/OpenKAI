/*
 * ROS_fastLio.cpp
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#include "ROS_fastLio.h"

namespace kai
{
    bool ROS_fastLio::init(const json &j)
    {
        IF_F(!j.is_object());

        jKv(j, "topicPC2", m_topicPC2);
        jKv(j, "topicOdom", m_topicOdom);
        jKv(j, "topicPath", m_topicPath);

        return true;
    }

    bool ROS_fastLio::link(const json &j, ModuleMgr *pM)
    {
        return true;
    }

    int ROS_fastLio::createSubscriptions(void)
    {
        if (!m_topicPC2.empty())
        {
            rclcpp::QoS qos(40);
            // qos.best_effort();
            // qos.durability_volatile();

            m_pScPC2 = this->create_subscription<sensor_msgs::msg::PointCloud2>(
                m_topicPC2,
                qos,
                std::bind(&ROS_fastLio::cbPointCloud2, this, _1));
        }

        if (!m_topicOdom.empty())
        {
            m_pScOdometry = this->create_subscription<nav_msgs::msg::Odometry>(
                m_topicOdom,
                40,
                std::bind(&ROS_fastLio::cbOdometry, this, _1));
        }

        if (!m_topicPath.empty())
        {
            m_pScPath = this->create_subscription<nav_msgs::msg::Path>(
                m_topicPath,
                40,
                std::bind(&ROS_fastLio::cbPath, this, _1));
        }

        return true;
    }

    void ROS_fastLio::cbPointCloud2(const sensor_msgs::msg::PointCloud2::UniquePtr pMsg)
    {
        //        LOG_("received PointCloud2 ");

#ifdef WITH_3D
        NULL_(m_pPCframe);

        sensor_msgs::PointCloud2Iterator<float> iter_x(*pMsg, "x");
        sensor_msgs::PointCloud2Iterator<float> iter_y(*pMsg, "y");
        sensor_msgs::PointCloud2Iterator<float> iter_z(*pMsg, "z");

        int nP = pMsg->height * pMsg->width;
        PointCloud *m_pPC = m_pPCframe->getNextBuffer();

        for (int i = 0; i < nP; i++, ++iter_x, ++iter_y, ++iter_z)
        {
            Vector3d vP(*iter_x, *iter_y, *iter_z);
            m_pPC->points_.push_back(vP);

            Vector3d vC(1, 1, 1);
            m_pPC->colors_.push_back(vC);
        }

        m_pPCframe->swapBuffer();

#endif
    }

    void ROS_fastLio::cbOdometry(const nav_msgs::msg::Odometry::UniquePtr pMsg)
    {
        m_vP.x = pMsg->pose.pose.position.x;
        m_vP.y = pMsg->pose.pose.position.y;
        m_vP.z = pMsg->pose.pose.position.z;

        m_vQ.x = pMsg->pose.pose.orientation.x;
        m_vQ.y = pMsg->pose.pose.orientation.y;
        m_vQ.z = pMsg->pose.pose.orientation.z;
        m_vQ.w = pMsg->pose.pose.orientation.w;

        m_vA.x = 0;
        m_vA.y = 0;
        m_vA.z = 0;

        Matrix3f mR;
        mR = Eigen::Quaternionf(
                 m_vQ.w,
                 m_vQ.x,
                 m_vQ.y,
                 m_vQ.z)
                 .toRotationMatrix();

        Matrix4f mT = Matrix4f::Identity();
        mT.block(0, 0, 3, 3) = mR;
        mT(0, 3) = m_vP.x;
        mT(1, 3) = m_vP.y;
        mT(2, 3) = m_vP.z;
        m_mT = mT;
    }

    void ROS_fastLio::cbPath(const nav_msgs::msg::Path::UniquePtr pMsg)
    {
    }

    void ROS_fastLio::console(void *pConsole)
    {
        NULL_(pConsole);

        _Console *pC = (_Console *)pConsole;
        pC->addMsg("vP = (" + f2str(m_vP.x) + ", " + f2str(m_vP.y) + ", " + f2str(m_vP.z) + ")");
        pC->addMsg("vA = (" + f2str(m_vA.x) + ", " + f2str(m_vA.y) + ", " + f2str(m_vA.z) + ")");
        pC->addMsg("vQ = (" + f2str(m_vQ.x) + ", " + f2str(m_vQ.y) + ", " + f2str(m_vQ.z) + ", " + f2str(m_vQ.w) + ")");
    }
}
