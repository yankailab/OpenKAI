/*
 * MavSetAttitudeTarget.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavSetAttitudeTarget.h"

namespace kai
{

	MavSetAttitudeTarget::MavSetAttitudeTarget()
	{
		m_id = MAVLINK_MSG_ID_SET_ATTITUDE_TARGET;
	}

	const MAV_MSG_TSTAMP& MavSetAttitudeTarget::add(mavlink_set_attitude_target_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_set_attitude_target_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavSetAttitudeTarget::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_set_attitude_target_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
