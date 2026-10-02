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

		clearMsgQueue();
	}

	void MavGlobalVisionPositionEstimate::add(mavlink_global_vision_position_estimate_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_global_vision_position_estimate_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavGlobalVisionPositionEstimate::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_global_vision_position_estimate_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
