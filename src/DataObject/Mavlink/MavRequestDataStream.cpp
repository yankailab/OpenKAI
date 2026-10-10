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
	}

	const MAV_MSG_TSTAMP& MavRequestDataStream::add(mavlink_request_data_stream_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_request_data_stream_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavRequestDataStream::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_request_data_stream_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
