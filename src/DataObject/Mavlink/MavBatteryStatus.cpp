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

		clearMsgQueue();
	}

	void MavBatteryStatus::add(mavlink_battery_status_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_battery_status_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavBatteryStatus::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_battery_status_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
