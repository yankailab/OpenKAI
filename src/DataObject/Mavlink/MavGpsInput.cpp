/*
 * MavGpsInput.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavGpsInput.h"

namespace kai
{

	MavGpsInput::MavGpsInput()
	{
		m_id = MAVLINK_MSG_ID_GPS_INPUT;
	}

	const MAV_MSG_TSTAMP& MavGpsInput::add(mavlink_gps_input_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_gps_input_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavGpsInput::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_gps_input_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
