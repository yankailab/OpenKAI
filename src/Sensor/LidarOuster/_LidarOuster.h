/*
 * _RPLIDAR.h
 *
 *  Created on: Apr 3, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__LidarOuster_H_
#define OpenKAI_src_Sensor__LidarOuster_H_

#include "../../Base/_ThreadBase.h"
#include "os1.h"
#include "os1_packet.h"

namespace kai
{
using namespace ouster::OS1;

class _LidarOuster : public _ThreadBase
{
  public:
	_LidarOuster();
	~_LidarOuster();

	bool init(void *pKiss);
	bool start(void);
	//bool draw(void);
	bool console(int &iY);

  private:
	bool open(void);
	void update(void);
	void updateLidar(void);
	static void *getUpdateThread(void *This)
	{
		((_LidarOuster *)This)->update();
		return NULL;
	}

	void handle_lidar(uint8_t *buf);
	void handle_imu(uint8_t *buf);

  public:
	// To get the ip address of lidar, run in shell:
	// nmap --open -p T:7501 192.168.100.0/24
	string m_hostname;		// ip address of the lidar;
	string m_udp_dest_host; // ip address of the host;
	std::shared_ptr<client> m_cli;
	bool m_flag_open = false;

	// Lidar;
	uint64_t m_n_lidar_packets = 0; // package number;
	uint64_t m_lidar_col_0_ts = 0;  // timestamp;
	float m_lidar_col_0_h_angle = 0.0; // angle for the col 0;
	uint8_t m_lidar_buf[lidar_packet_bytes + 1];

	// IMU
	uint64_t m_n_imu_packets = 0; // package number;
	uint64_t m_imu_ts = 0;		  // timestamp;
	float m_imu_av_z = 0.0;		  // angular velocity z;
	float m_imu_la_y = 0.0;		  // linear acceleration y;
	uint8_t m_imu_buf[imu_packet_bytes + 1];
};

} // namespace kai
#endif
