/*
 * MavSetPositionTargetGlobalINT.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavSetPositionTargetGlobalINT.h"

namespace kai
{

	MavSetPositionTargetGlobalINT::MavSetPositionTargetGlobalINT()
	{
		m_id = MAVLINK_MSG_ID_SET_POSITION_TARGET_GLOBAL_INT;

		clearMsgQueue();
	}

	void MavSetPositionTargetGlobalINT::add(mavlink_set_position_target_global_int_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_set_position_target_global_int_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavSetPositionTargetGlobalINT::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_set_position_target_global_int_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
