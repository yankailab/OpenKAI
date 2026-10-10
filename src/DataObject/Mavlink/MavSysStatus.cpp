/*
 * MavSysStatus.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavSysStatus.h"

namespace kai
{

	MavSysStatus::MavSysStatus()
	{
		m_id = MAVLINK_MSG_ID_SYS_STATUS;
	}

	const MAV_MSG_TSTAMP& MavSysStatus::add(mavlink_sys_status_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_sys_status_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavSysStatus::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_sys_status_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
