/*
 * _ROSbase.h
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_ROS_ROS_fastLio_H_
#define OpenKAI_src_ROS_ROS_fastLio_H_

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
using namespace std;
using std::placeholders::_1;

#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/point_cloud2_iterator.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "nav_msgs/msg/path.hpp"

#include "../UI/_Console.h"
#ifdef WITH_UNIVERSE
#include "../Universe/Geometry/PointCloud/_PCframe.h"
#endif

namespace kai
{
	class ROS_fastLio : public rclcpp::Node
	{
	public:
		ROS_fastLio() : Node("openkai_node")
		{
			m_vAxisIdx = Vector3i(0, 1, 2);
			m_vP.setZero();
			m_vA.setZero();
			m_vQ.setZero();

			m_mT = Eigen::Matrix4f::Identity();
		}

		virtual bool init(const json &j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual void console(void *pConsole);

		int createSubscriptions(void);

	protected:
		void cbPointCloud2(const sensor_msgs::msg::PointCloud2::UniquePtr pMsg);
		void cbOdometry(const nav_msgs::msg::Odometry::UniquePtr pMsg);
		void cbPath(const nav_msgs::msg::Path::UniquePtr pMsg);

	protected:
		rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr m_pScPC2;
		rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr m_pScOdometry;
		rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr m_pScPath;

	public:
		string m_topicPC2 = ""; // "Laser_map";
		string m_topicOdom = ""; // "Odometry";
		string m_topicPath = ""; // "path";

		Vector3i m_vAxisIdx = Vector3i::Zero();
		Vector3f m_vP = Vector3f::Zero();
		Vector3f m_vA = Vector3f::Zero();
		Vector4f m_vQ = Vector4f::Zero();
		Eigen::Matrix4f m_mT;

	protected:

#ifdef WITH_UNIVERSE
	public:
		_PCframe *m_pPCframe = NULL;
#endif
	};

}
#endif
