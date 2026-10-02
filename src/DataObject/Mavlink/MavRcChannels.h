/*
 * MavRcChannels.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavRcChannels__H_
#define OpenKAI_src__DataObject__Mavlink__MavRcChannels__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavRcChannels : public MavMsgBase
	{
	public:
		uint16_t *m_pChan[19] = {
			NULL, &m_msg.chan1_raw, &m_msg.chan2_raw, &m_msg.chan3_raw,
			&m_msg.chan4_raw, &m_msg.chan5_raw, &m_msg.chan6_raw, &m_msg.chan7_raw,
			&m_msg.chan8_raw, &m_msg.chan9_raw, &m_msg.chan10_raw, &m_msg.chan11_raw,
			&m_msg.chan12_raw, &m_msg.chan13_raw, &m_msg.chan14_raw, &m_msg.chan15_raw,
			&m_msg.chan16_raw, &m_msg.chan17_raw, &m_msg.chan18_raw};

		MavRcChannels();

		// for send message to IO
		void add(mavlink_rc_channels_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

		uint16_t getRC(int iChan);

	protected:
		mavlink_rc_channels_t m_msg{};
	};

}
#endif
