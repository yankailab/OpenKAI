/*
 * MavMissionCurrent.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavMissionCurrent.h"

namespace kai
{

	MavMissionCurrent::MavMissionCurrent()
	{
		m_id = MAVLINK_MSG_ID_MISSION_CURRENT;

		clearMsgQueue();
	}

	void MavMissionCurrent::add(mavlink_mission_current_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_mission_current_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavMissionCurrent::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mission_current_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
