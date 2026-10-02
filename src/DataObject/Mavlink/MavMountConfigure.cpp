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

		clearMsgQueue();
	}

	void MavMountConfigure::add(mavlink_mount_configure_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_mount_configure_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavMountConfigure::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mount_configure_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
