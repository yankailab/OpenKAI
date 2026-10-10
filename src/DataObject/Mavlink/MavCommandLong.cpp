/*
 * MavCommandLong.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavCommandLong.h"

namespace kai
{

	MavCommandLong::MavCommandLong()
	{
		m_id = MAVLINK_MSG_ID_COMMAND_LONG;
	}

	const MAV_MSG_TSTAMP& MavCommandLong::add(mavlink_command_long_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_command_long_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavCommandLong::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_command_long_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
