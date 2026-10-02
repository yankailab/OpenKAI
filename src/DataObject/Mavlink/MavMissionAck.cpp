/*
 * MavMissionAck.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavMissionAck.h"

namespace kai
{

	MavMissionAck::MavMissionAck()
	{
		m_id = MAVLINK_MSG_ID_MISSION_ACK;

		clearMsgQueue();
	}

	void MavMissionAck::add(mavlink_mission_ack_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_mission_ack_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavMissionAck::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mission_ack_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
