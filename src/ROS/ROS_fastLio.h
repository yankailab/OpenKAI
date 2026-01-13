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

#include "../Module/Kiss.h"
#include "../UI/_Console.h"
#ifdef WITH_3D
#include "../3D/PointCloud/_PCframe.h"
#endif
using namespace Eigen;

namespace kai
{
	class ROS_fastLio : public rclcpp::Node
	{
	public:
		ROS_fastLio() : Node("openkai_node")
		{
			m_topicPC2 = "";  //"Laser_map";
			m_topicOdom = ""; //"Odometry";
			m_topicPath = ""; //"path";

			m_vAxisIdx.set(0, 1, 2);
			m_vP.set(0);
			m_vA.set(0);
			m_vQ.set(0);

			m_mT = Matrix4f::Identity();
		}

		int init(Kiss *pKiss);
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
		string m_topicPC2;
		string m_topicOdom;
		string m_topicPath;

		vInt3 m_vAxisIdx;
		vFloat3 m_vP;
		vFloat3 m_vA;
		vFloat4 m_vQ;
		Matrix4f m_mT;

	protected:
		Kiss *m_pKiss;

#ifdef WITH_3D
	public:
		_PCframe *m_pPCframe = NULL;
#endif
	};

}
#endif
