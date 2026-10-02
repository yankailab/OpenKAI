/*
 * MavMountConfigure.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavMountConfigure__H_
#define OpenKAI_src__DataObject__Mavlink__MavMountConfigure__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavMountConfigure : public MavMsgBase
	{
	public:
		MavMountConfigure();

		// for send message to IO
		void add(mavlink_mount_configure_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

	protected:
		mavlink_mount_configure_t m_msg{};
	};

}
#endif
