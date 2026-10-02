/*
 * MavGpsRawINT.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavGpsRawINT.h"

namespace kai
{

	MavGpsRawINT::MavGpsRawINT()
	{
		m_id = MAVLINK_MSG_ID_GPS_RAW_INT;

		clearMsgQueue();
	}

	void MavGpsRawINT::add(mavlink_gps_raw_int_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_gps_raw_int_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavGpsRawINT::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_gps_raw_int_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
