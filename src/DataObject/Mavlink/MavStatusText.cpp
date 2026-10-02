/*
 * MavStatusText.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavStatusText.h"

namespace kai
{

	MavStatusText::MavStatusText()
	{
		m_id = MAVLINK_MSG_ID_STATUSTEXT;

		clearMsgQueue();
	}

	void MavStatusText::add(mavlink_statustext_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_statustext_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavStatusText::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_statustext_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
