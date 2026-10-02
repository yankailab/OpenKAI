/*
 * MavSetMode.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavSetMode.h"

namespace kai
{

	MavSetMode::MavSetMode()
	{
		m_id = MAVLINK_MSG_ID_SET_MODE;

		clearMsgQueue();
	}

	void MavSetMode::add(mavlink_set_mode_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_set_mode_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavSetMode::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_set_mode_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
