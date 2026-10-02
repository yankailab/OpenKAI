/*
 * MavSetPositionTargetLocalNED.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavSetPositionTargetLocalNED.h"

namespace kai
{

	MavSetPositionTargetLocalNED::MavSetPositionTargetLocalNED()
	{
		m_id = MAVLINK_MSG_ID_SET_POSITION_TARGET_LOCAL_NED;

		clearMsgQueue();
	}

	void MavSetPositionTargetLocalNED::add(mavlink_set_position_target_local_ned_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_set_position_target_local_ned_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavSetPositionTargetLocalNED::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_set_position_target_local_ned_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
