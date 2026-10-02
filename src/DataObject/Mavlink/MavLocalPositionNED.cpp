/*
 * MavLocalPositionNED.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavLocalPositionNED.h"

namespace kai
{

	MavLocalPositionNED::MavLocalPositionNED()
	{
		m_id = MAVLINK_MSG_ID_LOCAL_POSITION_NED;

		clearMsgQueue();
	}

	void MavLocalPositionNED::add(mavlink_local_position_ned_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_local_position_ned_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavLocalPositionNED::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_local_position_ned_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
