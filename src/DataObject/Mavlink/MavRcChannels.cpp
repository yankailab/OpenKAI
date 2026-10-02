/*
 * MavRcChannels.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavRcChannels.h"

namespace kai
{

	MavRcChannels::MavRcChannels()
	{
		m_id = MAVLINK_MSG_ID_RC_CHANNELS;

		m_msg.chancount = 0;
		m_msg.chan1_raw = UINT16_MAX;
		m_msg.chan2_raw = UINT16_MAX;
		m_msg.chan3_raw = UINT16_MAX;
		m_msg.chan4_raw = UINT16_MAX;
		m_msg.chan5_raw = UINT16_MAX;
		m_msg.chan6_raw = UINT16_MAX;
		m_msg.chan7_raw = UINT16_MAX;
		m_msg.chan8_raw = UINT16_MAX;
		m_msg.chan9_raw = UINT16_MAX;
		m_msg.chan10_raw = UINT16_MAX;
		m_msg.chan11_raw = UINT16_MAX;
		m_msg.chan12_raw = UINT16_MAX;
		m_msg.chan13_raw = UINT16_MAX;
		m_msg.chan14_raw = UINT16_MAX;
		m_msg.chan15_raw = UINT16_MAX;
		m_msg.chan16_raw = UINT16_MAX;
		m_msg.chan17_raw = UINT16_MAX;
		m_msg.chan18_raw = UINT16_MAX;
		m_msg.rssi = 255;

		clearMsgQueue();
	}

	void MavRcChannels::add(mavlink_rc_channels_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_rc_channels_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavRcChannels::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_rc_channels_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

	uint16_t MavRcChannels::getRC(int iChan)
	{
		if (iChan <= 0 || iChan > 18)
			return UINT16_MAX;

		return *m_pChan[iChan];
	}

}
