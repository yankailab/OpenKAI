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

		clearMsgQueue();
	}

	void MavMissionCount::add(mavlink_mission_count_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_mission_count_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavMissionCount::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mission_count_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
