/*
 * MavCommandAck.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavCommandAck.h"

namespace kai
{

	MavCommandAck::MavCommandAck()
	{
		m_id = MAVLINK_MSG_ID_COMMAND_ACK;
	}

	const MAV_MSG_TSTAMP& MavCommandAck::add(mavlink_command_ack_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_command_ack_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavCommandAck::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_command_ack_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
