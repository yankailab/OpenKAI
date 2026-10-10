/*
 * MavMissionItemInt.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavMissionItemInt.h"

namespace kai
{

	MavMissionItemInt::MavMissionItemInt()
	{
		m_id = MAVLINK_MSG_ID_MISSION_ITEM_INT;
	}

	const MAV_MSG_TSTAMP& MavMissionItemInt::add(mavlink_mission_item_int_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_mission_item_int_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavMissionItemInt::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mission_item_int_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
