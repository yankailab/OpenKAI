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

		clearMsgQueue();
	}

	void MavHighresIMU::add(mavlink_highres_imu_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_highres_imu_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavHighresIMU::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_highres_imu_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
