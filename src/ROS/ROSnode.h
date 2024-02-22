/*
 * _ROSbase.h
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_ROS_ROSnode_H_
#define OpenKAI_src_ROS_ROSnode_H_

#include "../Base/common.h"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std;
using std::placeholders::_1;

namespace kai
{
	class ROSnode : public rclcpp::Node
	{
	public:
		ROSnode() : Node("openkai_node")
		{
		}

		bool createSubscriptions(void);

	protected:
		void cbTopic(const std_msgs::msg::String &msg);

	protected:
		rclcpp::Subscription<std_msgs::msg::String>::SharedPtr m_pScTopic;

	};

}
#endif
