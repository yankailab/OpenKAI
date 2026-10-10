/*
 * MavMissionItemReached.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavMissionItemReached.h"

namespace kai
{

	MavMissionItemReached::MavMissionItemReached()
	{
		m_id = MAVLINK_MSG_ID_MISSION_ITEM_REACHED;
	}

	const MAV_MSG_TSTAMP& MavMissionItemReached::add(mavlink_mission_item_reached_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_mission_item_reached_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavMissionItemReached::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mission_item_reached_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
