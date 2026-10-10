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
	}

	const MAV_MSG_TSTAMP& MavHeartbeat::add(mavlink_heartbeat_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_heartbeat_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavHeartbeat::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_heartbeat_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
