/*
 * MavMountConfigure.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavMountConfigure.h"

namespace kai
{

	MavMountConfigure::MavMountConfigure()
	{
		m_id = MAVLINK_MSG_ID_MOUNT_CONFIGURE;
	}

	const MAV_MSG_TSTAMP& MavMountConfigure::add(mavlink_mount_configure_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_mount_configure_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavMountConfigure::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mount_configure_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
