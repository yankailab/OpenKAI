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
	}

	const MAV_MSG_TSTAMP& MavDistanceSensor::add(mavlink_distance_sensor_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_msg_distance_sensor_encode(mySysID, myComID, &m_msgT.m_msgT, &msg);

		return m_msgT;
	}

	void MavDistanceSensor::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_distance_sensor_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

}
