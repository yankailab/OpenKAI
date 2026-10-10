/*
 * MavBatteryStatus.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavBatteryStatus.h"

namespace kai
{

	MavBatteryStatus::MavBatteryStatus()
	{
		m_id = MAVLINK_MSG_ID_BATTERY_STATUS;
	}

	const MAV_MSG_TSTAMP& MavBatteryStatus::add(mavlink_battery_status_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_battery_status_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavBatteryStatus::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_battery_status_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
