/*
 * MavGpsRTCMdata.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavGpsRTCMdata.h"

namespace kai
{

	MavGpsRTCMdata::MavGpsRTCMdata()
	{
		m_id = MAVLINK_MSG_ID_GPS_RTCM_DATA;
	}

	const MAV_MSG_TSTAMP& MavGpsRTCMdata::add(mavlink_gps_rtcm_data_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_gps_rtcm_data_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavGpsRTCMdata::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_gps_rtcm_data_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
