/*
 * MavMissionRequestInt.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavMissionRequestInt.h"

namespace kai
{

	MavMissionRequestInt::MavMissionRequestInt()
	{
		m_id = MAVLINK_MSG_ID_MISSION_REQUEST_INT;
	}

	const MAV_MSG_TSTAMP& MavMissionRequestInt::add(mavlink_mission_request_int_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_mission_request_int_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavMissionRequestInt::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mission_request_int_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
