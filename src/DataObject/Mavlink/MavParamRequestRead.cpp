/*
 * MavParamRequestRead.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavParamRequestRead.h"

namespace kai
{

	MavParamRequestRead::MavParamRequestRead()
	{
		m_id = MAVLINK_MSG_ID_PARAM_REQUEST_READ;

		clearMsgQueue();
	}

	void MavParamRequestRead::add(mavlink_param_request_read_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_param_request_read_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavParamRequestRead::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_param_request_read_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
