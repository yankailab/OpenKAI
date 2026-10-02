/*
 * MavPositionTargetLocalNED.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavPositionTargetLocalNED.h"

namespace kai
{

	MavPositionTargetLocalNED::MavPositionTargetLocalNED()
	{
		m_id = MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED;

		clearMsgQueue();
	}

	void MavPositionTargetLocalNED::add(mavlink_position_target_local_ned_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_position_target_local_ned_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavPositionTargetLocalNED::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_position_target_local_ned_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
