/*
 * MavButtonChange.cpp
 *
 *  Created on: Oct 27, 2026
 */

#include "MavButtonChange.h"

namespace kai
{

	MavButtonChange::MavButtonChange()
	{
		m_id = MAVLINK_MSG_ID_BUTTON_CHANGE;
	}

	const MAV_MSG_TSTAMP& MavButtonChange::add(mavlink_button_change_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_button_change_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavButtonChange::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_button_change_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
