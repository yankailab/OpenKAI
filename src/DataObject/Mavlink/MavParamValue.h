/*
 * MavParamValue.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavParamValue__H_
#define OpenKAI_src__DataObject__Mavlink__MavParamValue__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavParamValue : public MavMsgBase
	{
	public:
		MavParamValue();

		// for send message to IO
		void add(mavlink_param_value_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

	protected:
		mavlink_param_value_t m_msg{};
	};

}
#endif
