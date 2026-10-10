/*
 * MavLandingTarget.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavLandingTarget.h"

namespace kai
{

	MavLandingTarget::MavLandingTarget()
	{
		m_id = MAVLINK_MSG_ID_LANDING_TARGET;
	}

	const MAV_MSG_TSTAMP& MavLandingTarget::add(mavlink_landing_target_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_landing_target_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavLandingTarget::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_landing_target_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
