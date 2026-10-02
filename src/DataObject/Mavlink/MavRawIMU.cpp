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

		clearMsgQueue();
	}

	void MavRawIMU::add(mavlink_raw_imu_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_raw_imu_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavRawIMU::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_raw_imu_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
