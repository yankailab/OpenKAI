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
	}

	const MAV_MSG_TSTAMP& MavSetMode::add(mavlink_set_mode_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_set_mode_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavSetMode::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_set_mode_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
