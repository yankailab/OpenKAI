/*
 * MavAttitude.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavAttitude__H_
#define OpenKAI_src__DataObject__Mavlink__MavAttitude__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavAttitude : public MavMsgBase
	{
	public:
		MavAttitude();

		// for send message to IO
		void add(mavlink_attitude_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

	protected:
		mavlink_attitude_t m_msg{};
	};

}
#endif
