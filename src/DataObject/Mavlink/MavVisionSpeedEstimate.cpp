/*
 * MavVisionSpeedEstimate.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavVisionSpeedEstimate.h"

namespace kai
{

	MavVisionSpeedEstimate::MavVisionSpeedEstimate()
	{
		m_id = MAVLINK_MSG_ID_VISION_SPEED_ESTIMATE;
	}

	const MAV_MSG_TSTAMP& MavVisionSpeedEstimate::add(mavlink_vision_speed_estimate_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_vision_speed_estimate_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavVisionSpeedEstimate::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_vision_speed_estimate_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
