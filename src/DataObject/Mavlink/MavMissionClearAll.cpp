/*
 * MavMissionClearAll.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavMissionClearAll.h"

namespace kai
{

	MavMissionClearAll::MavMissionClearAll()
	{
		m_id = MAVLINK_MSG_ID_MISSION_CLEAR_ALL;

		clearMsgQueue();
	}

	void MavMissionClearAll::add(mavlink_mission_clear_all_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_mission_clear_all_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavMissionClearAll::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mission_clear_all_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
