/*
 * _ROSbase.h
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_ROS_ROSbase_H_
#define OpenKAI_src_ROS_ROSbase_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
using std::placeholders::_1;

namespace kai
{
	class ROSsubscriber : public rclcpp::Node
	{
	public:
		ROSsubscriber() : Node("openkai_subscriber")
		{
			subscription_ = this->create_subscription<std_msgs::msg::String>(
				"topic",
				10,
				std::bind(&ROSsubscriber::cbTopic,
				this,
				_1));
		}

	private:
		void cbTopic(const std_msgs::msg::String &msg) const
		{
			LOG_(msg.data.c_str());
		}

		rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
	};

	class _ROSbase : public _ModuleBase
	{
	public:
		_ROSbase();
		virtual ~_ROSbase();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual int check(void);
		virtual bool start(void);
		virtual void console(void *pConsole);

	protected:
		void updateROS(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ROSbase *)This)->update();
			return NULL;
		}

	protected:

	};

}
#endif
