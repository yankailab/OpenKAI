/*
 *  Created on: Nov 24, 2017
 *      Author: yankai
 */
#include "_LidarOuster.h"

namespace kai
{

_LidarOuster::_LidarOuster()
{
	m_hostname = "";
	m_udp_dest_host = "";
}

_LidarOuster::~_LidarOuster()
{
	m_bThreadON = false;
}

void _LidarOuster::handle_lidar(uint8_t *buf)
{
	m_n_lidar_packets++;
	m_lidar_col_0_ts = col_timestamp(nth_col(0, buf));
	m_lidar_col_0_h_angle = col_h_angle(nth_col(0, buf));
}

void _LidarOuster::handle_imu(uint8_t *buf)
{
	m_n_imu_packets++;
	m_imu_ts = imu_sys_ts(buf);
	m_imu_av_z = imu_av_z(buf);
	m_imu_la_y = imu_la_y(buf);
}

bool _LidarOuster::init(void *pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss *pK = (Kiss *)pKiss;

	KISSm(pK, hostname);
	KISSm(pK, udp_dest_host);
	return true;
}

bool _LidarOuster::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _LidarOuster::update(void)
{
	while (m_bThreadON)
	{
		if (!m_flag_open)
		{
			m_flag_open = open();
			if (!m_flag_open)
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();
		updateLidar();
		this->autoFPSto();
	}
}

bool _LidarOuster::open(void)
{
	m_cli = init_client(m_hostname, m_udp_dest_host, 7502, 7503);
	if (!m_cli)
	{
		std::cerr << "Failed to connect to : " << m_hostname << std::endl;
		return false;
	}
	return true;
}

void _LidarOuster::updateLidar(void)
{
	client_state st = poll_client(*m_cli);
	if (st & ERROR)
	{
		std::cerr << "Fail to update Lidar" << std::endl;
	}
	else if (st & LIDAR_DATA)
	{
		if (read_lidar_packet(*m_cli, m_lidar_buf))
			handle_lidar(m_lidar_buf);
	}
	else if (st & IMU_DATA)
	{
		if (read_imu_packet(*m_cli, m_imu_buf))
			handle_imu(m_imu_buf);
	}
}

bool _LidarOuster::console(int &iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;
	msg += "n_lidar_packets=" + li2str(m_n_lidar_packets);
	msg += "; col_0_azimuth=" + f2str(m_lidar_col_0_h_angle, 6);
	msg += "; col_0_ts=" + li2str(m_lidar_col_0_ts);
	
	COL_MSG;
	iY++;
	mvaddstr(iY, CONSOLE_X_MSG, msg.c_str());

	msg = "";
	msg += "n_imu_packets=" + li2str(m_n_imu_packets);
	msg += "; im_av_z=" + f2str(m_imu_av_z, 6);
	msg += "; im_la_y=" + f2str(m_imu_la_y, 6);
	msg += "; imu_ts=" + li2str(m_imu_ts);

	COL_MSG;
	iY++;
	mvaddstr(iY, CONSOLE_X_MSG, msg.c_str());

	return true;
}

} // namespace kai
