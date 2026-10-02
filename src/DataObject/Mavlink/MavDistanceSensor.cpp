/*
 * MavDistanceSensor.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavDistanceSensor.h"

namespace kai
{

	MavDistanceSensor::MavDistanceSensor()
	{
		m_id = MAVLINK_MSG_ID_DISTANCE_SENSOR;

		clearMsgQueue();
	}

	void MavDistanceSensor::add(mavlink_distance_sensor_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_distance_sensor_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavDistanceSensor::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_distance_sensor_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
