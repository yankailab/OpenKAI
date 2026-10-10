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
	}

	const MAV_MSG_TSTAMP& MavMissionAck::add(mavlink_mission_ack_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_mission_ack_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavMissionAck::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mission_ack_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
