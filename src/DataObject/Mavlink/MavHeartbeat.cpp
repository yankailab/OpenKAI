/*
 * MavHeartbeat.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavHeartbeat.h"

namespace kai
{

	MavHeartbeat::MavHeartbeat()
	{
		m_id = MAVLINK_MSG_ID_HEARTBEAT;

		clearMsgQueue();
	}

	void MavHeartbeat::add(mavlink_heartbeat_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_heartbeat_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavHeartbeat::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_heartbeat_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
