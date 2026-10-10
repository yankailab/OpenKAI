/*
 * MavParamValue.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavParamValue.h"

namespace kai
{

	MavParamValue::MavParamValue()
	{
		m_id = MAVLINK_MSG_ID_PARAM_VALUE;
	}

	const MAV_MSG_TSTAMP& MavParamValue::add(mavlink_param_value_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_param_value_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavParamValue::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_param_value_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
