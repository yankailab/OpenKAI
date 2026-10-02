/*
 * MavMountStatus.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavMountStatus.h"

namespace kai
{

	MavMountStatus::MavMountStatus()
	{
		m_id = MAVLINK_MSG_ID_MOUNT_STATUS;

		clearMsgQueue();
	}

	void MavMountStatus::add(mavlink_mount_status_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_mount_status_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavMountStatus::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_mount_status_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
