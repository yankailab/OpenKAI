/*
 * MavMissionCount.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavMissionCount.h"

namespace kai
{

	MavMissionCount::MavMissionCount()
	{
		m_id = MAVLINK_MSG_ID_MISSION_COUNT;
	}

	const MAV_MSG_TSTAMP& MavMissionCount::add(mavlink_mission_count_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_mission_count_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavMissionCount::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mission_count_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
