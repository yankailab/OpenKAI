/*
 * MavGlobalVisionPositionEstimate.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavGlobalVisionPositionEstimate.h"

namespace kai
{

	MavGlobalVisionPositionEstimate::MavGlobalVisionPositionEstimate()
	{
		m_id = MAVLINK_MSG_ID_GLOBAL_VISION_POSITION_ESTIMATE;
	}

	const MAV_MSG_TSTAMP& MavGlobalVisionPositionEstimate::add(mavlink_global_vision_position_estimate_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_global_vision_position_estimate_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavGlobalVisionPositionEstimate::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_global_vision_position_estimate_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
