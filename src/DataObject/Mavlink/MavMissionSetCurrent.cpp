/*
 * MavMissionSetCurrent.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavMissionSetCurrent.h"

namespace kai
{

	MavMissionSetCurrent::MavMissionSetCurrent()
	{
		m_id = MAVLINK_MSG_ID_MISSION_SET_CURRENT;
	}

	const MAV_MSG_TSTAMP& MavMissionSetCurrent::add(mavlink_mission_set_current_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_mission_set_current_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavMissionSetCurrent::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mission_set_current_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
