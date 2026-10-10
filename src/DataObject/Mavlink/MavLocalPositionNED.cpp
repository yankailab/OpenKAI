/*
 * MavLocalPositionNED.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavLocalPositionNED.h"

namespace kai
{

	MavLocalPositionNED::MavLocalPositionNED()
	{
		m_id = MAVLINK_MSG_ID_LOCAL_POSITION_NED;
	}

	const MAV_MSG_TSTAMP& MavLocalPositionNED::add(mavlink_local_position_ned_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_local_position_ned_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavLocalPositionNED::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_local_position_ned_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
