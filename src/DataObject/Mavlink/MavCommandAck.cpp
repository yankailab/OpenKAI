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

		clearMsgQueue();
	}

	void MavCommandAck::add(mavlink_command_ack_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_command_ack_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavCommandAck::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_command_ack_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
