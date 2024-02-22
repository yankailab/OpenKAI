/*
 * _ROSbase.h
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_ROS_ROS_fastLio_H_
#define OpenKAI_src_ROS_ROS_fastLio_H_

#include "ROSnode.h"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/point_cloud2_iterator.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "nav_msgs/msg/path.hpp"

#ifdef WITH_3D
#include "../3D/PointCloud/_PCframe.h"
#endif

namespace kai
{
	class ROS_fastLio : public rclcpp::Node
	{
	public:
		ROS_fastLio() : Node("openkai_node")
		{
			m_topic = "";
		}

		bool createSubscriptions(void);

	protected:
		void cbPointCloud2(const sensor_msgs::msg::PointCloud2::UniquePtr pMsg);
		void cbOdometry(const nav_msgs::msg::Odometry::UniquePtr pMsg);
		void cbPath(const nav_msgs::msg::Path::UniquePtr pMsg);

	protected:
		rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr m_pScPC2;
		rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr m_pScOdometry;
		rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr m_pScPath;

	public:
		string m_topic;

#ifdef WITH_3D
	public:
		_PCframe* m_pPCframe = NULL;
#endif

	};

}
#endif
