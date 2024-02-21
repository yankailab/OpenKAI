/*
 * _ROSbase.h
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_ROS_ROSnode_H_
#define OpenKAI_src_ROS_ROSnode_H_

#include "../Base/common.h"
using namespace std;

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
using std::placeholders::_1;

namespace kai
{
	class ROSnode : public rclcpp::Node
	{
	public:
		ROSnode() : Node("openkai_node")
		{
			m_subTopic = this->create_subscription<std_msgs::msg::String>(
				"topic",
				10,
				std::bind(&ROSnode::sCbTopic, this, _1));

			m_subPC2 = this->create_subscription<sensor_msgs::msg::PointCloud2>(
				"/livox/lidar",
				20,
				std::bind(&ROSnode::sCbPointCloud2, this, _1));
		}

	public:
		void sCbTopic(const std_msgs::msg::String &msg) const
		{
			LOG_(msg.data.c_str());
		}

		void sCbPointCloud2(const sensor_msgs::msg::PointCloud2::UniquePtr msg) const
		{
			LOG_("received PointCloud msg");
		}

	private:
		rclcpp::Subscription<std_msgs::msg::String>::SharedPtr m_subTopic;
		rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr m_subPC2;
	};

}
#endif
