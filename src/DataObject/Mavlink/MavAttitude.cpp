/*
 * MavAttitude.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavAttitude.h"

namespace kai
{

	MavAttitude::MavAttitude()
	{
		m_id = MAVLINK_MSG_ID_ATTITUDE;
	}

	const MAV_MSG_TSTAMP& MavAttitude::add(mavlink_attitude_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_attitude_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavAttitude::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_attitude_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
