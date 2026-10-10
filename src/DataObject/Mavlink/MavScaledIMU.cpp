/*
 * MavScaledIMU.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavScaledIMU.h"

namespace kai
{

	MavScaledIMU::MavScaledIMU()
	{
		m_id = MAVLINK_MSG_ID_SCALED_IMU;
	}

	const MAV_MSG_TSTAMP& MavScaledIMU::add(mavlink_scaled_imu_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_scaled_imu_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavScaledIMU::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_scaled_imu_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
