/*
 * MavRcChannelsOverride.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavRcChannelsOverride.h"

namespace kai
{

	MavRcChannelsOverride::MavRcChannelsOverride()
	{
		m_id = MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE;
	}

	const MAV_MSG_TSTAMP& MavRcChannelsOverride::add(mavlink_rc_channels_override_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_rc_channels_override_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavRcChannelsOverride::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_rc_channels_override_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
