/*
 * MavCommandInt.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavCommandInt.h"

namespace kai
{

	MavCommandInt::MavCommandInt()
	{
		m_id = MAVLINK_MSG_ID_COMMAND_INT;
	}

	const MAV_MSG_TSTAMP& MavCommandInt::add(mavlink_command_int_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_command_int_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavCommandInt::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_command_int_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
