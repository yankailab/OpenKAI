/*
 * MavAttitudeQuaternion.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavAttitudeQuaternion.h"

namespace kai
{

	MavAttitudeQuaternion::MavAttitudeQuaternion()
	{
		m_id = MAVLINK_MSG_ID_ATTITUDE_QUATERNION;
	}

	const MAV_MSG_TSTAMP& MavAttitudeQuaternion::add(mavlink_attitude_quaternion_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_attitude_quaternion_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavAttitudeQuaternion::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_attitude_quaternion_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
