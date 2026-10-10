/*
 * MavMountControl.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavMountControl.h"

namespace kai
{

	MavMountControl::MavMountControl()
	{
		m_id = MAVLINK_MSG_ID_MOUNT_CONTROL;
	}

	const MAV_MSG_TSTAMP& MavMountControl::add(mavlink_mount_control_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_mount_control_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavMountControl::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mount_control_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
