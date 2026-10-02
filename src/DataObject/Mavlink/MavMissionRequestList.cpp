/*
 * MavMissionRequestList.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavMissionRequestList.h"

namespace kai
{

	MavMissionRequestList::MavMissionRequestList()
	{
		m_id = MAVLINK_MSG_ID_MISSION_REQUEST_LIST;

		m_msg.mission_type = 0;

		clearMsgQueue();
	}

	void MavMissionRequestList::add(mavlink_mission_request_list_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_mission_request_list_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavMissionRequestList::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mission_request_list_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
