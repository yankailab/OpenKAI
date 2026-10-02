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

		clearMsgQueue();
	}

	void MavMissionSetCurrent::add(mavlink_mission_set_current_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_mission_set_current_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavMissionSetCurrent::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mission_set_current_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
