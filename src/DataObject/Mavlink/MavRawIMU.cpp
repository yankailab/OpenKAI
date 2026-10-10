/*
 * MavRawIMU.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavRawIMU.h"

namespace kai
{

	MavRawIMU::MavRawIMU()
	{
		m_id = MAVLINK_MSG_ID_RAW_IMU;
	}

	const MAV_MSG_TSTAMP& MavRawIMU::add(mavlink_raw_imu_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_raw_imu_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavRawIMU::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_raw_imu_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
