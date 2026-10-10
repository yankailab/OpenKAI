/*
 * MavParamSet.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavParamSet.h"

namespace kai
{

	MavParamSet::MavParamSet()
	{
		m_id = MAVLINK_MSG_ID_PARAM_SET;
	}

	const MAV_MSG_TSTAMP& MavParamSet::add(mavlink_param_set_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_param_set_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavParamSet::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_param_set_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
