/*
 * MavRadioStatus.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavRadioStatus.h"

namespace kai
{

	MavRadioStatus::MavRadioStatus()
	{
		m_id = MAVLINK_MSG_ID_RADIO_STATUS;

		clearMsgQueue();
	}

	void MavRadioStatus::add(mavlink_radio_status_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_radio_status_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavRadioStatus::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_radio_status_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
