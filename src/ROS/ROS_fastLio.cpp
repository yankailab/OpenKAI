/*
 * ROS_fastLio.cpp
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#include "ROS_fastLio.h"

namespace kai
{
    bool ROS_fastLio::createSubscriptions(void)
    {
        m_pScPC2 = this->create_subscription<sensor_msgs::msg::PointCloud2>(
//            "/livox/lidar",
            m_topic,//"/cloud_registered",
            40,
            std::bind(&ROS_fastLio::cbPointCloud2, this, _1));
    }

    void ROS_fastLio::cbPointCloud2(const sensor_msgs::msg::PointCloud2::UniquePtr pMsg)
    {
        LOG_("received PointCloud2 ");

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
    }

    void ROS_fastLio::cbPath(const nav_msgs::msg::Path::UniquePtr pMsg)
    {
    }

}
