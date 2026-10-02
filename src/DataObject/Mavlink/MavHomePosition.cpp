/*
 * MavHomePosition.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavHomePosition.h"

namespace kai
{

	MavHomePosition::MavHomePosition()
	{
		m_id = MAVLINK_MSG_ID_HOME_POSITION;

		clearMsgQueue();
	}

	void MavHomePosition::add(mavlink_home_position_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_home_position_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavHomePosition::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_home_position_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
