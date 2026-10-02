/*
 * MavMissionAck.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavMissionAck__H_
#define OpenKAI_src__DataObject__Mavlink__MavMissionAck__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavMissionAck : public MavMsgBase
	{
	public:
		MavMissionAck();

		// for send message to IO
		void add(mavlink_mission_ack_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

	protected:
		mavlink_mission_ack_t m_msg{};
	};

}
#endif
