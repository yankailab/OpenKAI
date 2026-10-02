/*
 * MavRequestDataStream.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavRequestDataStream.h"

namespace kai
{

	MavRequestDataStream::MavRequestDataStream()
	{
		m_id = MAVLINK_MSG_ID_REQUEST_DATA_STREAM;

		clearMsgQueue();
	}

	void MavRequestDataStream::add(mavlink_request_data_stream_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_request_data_stream_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavRequestDataStream::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_request_data_stream_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
