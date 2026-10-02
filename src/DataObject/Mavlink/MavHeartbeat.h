/*
 * MavHeartbeat.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavHeartbeat__H_
#define OpenKAI_src__DataObject__Mavlink__MavHeartbeat__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavHeartbeat : public MavMsgBase
	{
	public:
		MavHeartbeat();

		// for send message to IO
		void add(mavlink_heartbeat_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

	protected:
		mavlink_heartbeat_t m_msg{};
	};

}
#endif
