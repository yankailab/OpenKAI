/*
 * MavHighresIMU.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavHighresIMU.h"

namespace kai
{

	MavHighresIMU::MavHighresIMU()
	{
		m_id = MAVLINK_MSG_ID_HIGHRES_IMU;
	}

	const MAV_MSG_TSTAMP& MavHighresIMU::add(mavlink_highres_imu_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_highres_imu_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavHighresIMU::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_highres_imu_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
