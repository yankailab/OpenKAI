/*
 * MavGlobalPositionINT.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavGlobalPositionINT.h"

namespace kai
{

	MavGlobalPositionINT::MavGlobalPositionINT()
	{
		m_id = MAVLINK_MSG_ID_GLOBAL_POSITION_INT;

		clearMsgQueue();
	}

	void MavGlobalPositionINT::add(mavlink_global_position_int_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_global_position_int_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavGlobalPositionINT::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_global_position_int_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
