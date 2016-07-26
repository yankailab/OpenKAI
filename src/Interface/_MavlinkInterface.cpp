#include "../Utility/util.h"
#include "_MavlinkInterface.h"

namespace kai
{

_MavlinkInterface::_MavlinkInterface()
{
	_ThreadBase();

	m_bSerialOpen = false;
	m_sportName = "";
	m_pSerialPort = NULL;
	m_baudRate = 115200;

	current_messages.attitude.pitch = 0;
	current_messages.attitude.roll = 0;
	current_messages.attitude.yaw = 0;
	current_messages.attitude.pitchspeed = 0;
	current_messages.attitude.rollspeed = 0;
	current_messages.attitude.yawspeed = 0;

}

_MavlinkInterface::~_MavlinkInterface()
{
	if (m_pSerialPort)
	{
		delete m_pSerialPort;
	}
}

bool _MavlinkInterface::setup(JSON* pJson, string serialName)
{
	if(!pJson)return false;

	CHECK_ERROR(pJson->getVal("SERIALPORT_"+serialName+"_NAME", &m_sportName));
	CHECK_ERROR(pJson->getVal("SERIALPORT_"+serialName+"_BAUDRATE", &m_baudRate));

	double FPS = 500;
	CHECK_INFO(pJson->getVal("SERIALPORT_FC_FPS", &FPS));
	this->setTargetFPS(FPS);

	m_systemID = 1;
	m_componentID = MAV_COMP_ID_PATHPLANNER;
	m_type = MAV_TYPE_ONBOARD_CONTROLLER;
	m_lastHeartbeat = 0;
	m_targetComponentID = 0;

	current_messages.sysid = 0;
	current_messages.compid = 0;

	last_status.packet_rx_drop_count = 0;

	//Start Serial Port
	m_pSerialPort = new SerialPort();

	return true;
}

void _MavlinkInterface::close()
{
	if (m_pSerialPort)
	{
		delete m_pSerialPort;
		m_pSerialPort = NULL;
		m_bSerialOpen = false;
	}
	printf("Serial port closed.\n");
}

void _MavlinkInterface::handleMessages()
{
	mavlink_message_t message;
	int nMsgHandled;

	nMsgHandled = 0;

	//Handle Message while new message is received
	while (readMessage(message))
	{
		// Note this doesn't handle multiple message sources.
		current_messages.sysid = message.sysid;
		current_messages.compid = message.compid;

		// Handle Message ID
		switch (message.msgid)
		{

		case MAVLINK_MSG_ID_HEARTBEAT:
		{
#ifdef MAVLINK_DEBUG
			printf("MAVLINK_MSG_ID_HEARTBEAT\n");
#endif
			mavlink_msg_heartbeat_decode(&message,&(current_messages.heartbeat));
			current_messages.time_stamps.heartbeat = get_time_usec();

			if(current_messages.heartbeat.type != MAV_TYPE_GCS)
			{
				m_systemID = current_messages.sysid;
				m_targetComponentID = current_messages.compid;
#ifdef MAVLINK_DEBUG
				printf("      SYSTEM_ID: %d;  COMPONENT_ID: %d;  TARGET_COMPONENT_ID: %d;\n",m_systemID,m_componentID,m_targetComponentID);
#endif
			}
			else
			{
#ifdef MAVLINK_DEBUG
				printf("      RECEIVED HEARTBEAT FROM MAV_TYPE_GCS\n");
#endif
			}
			break;
		}

		case MAVLINK_MSG_ID_SYS_STATUS:
		{
#ifdef MAVLINK_DEBUG
			printf("MAVLINK_MSG_ID_SYS_STATUS\n");
#endif
			mavlink_msg_sys_status_decode(&message,
					&(current_messages.sys_status));
			current_messages.time_stamps.sys_status = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_BATTERY_STATUS:
		{
#ifdef MAVLINK_DEBUG
			printf("MAVLINK_MSG_ID_BATTERY_STATUS\n");
#endif
			mavlink_msg_battery_status_decode(&message,
					&(current_messages.battery_status));
			current_messages.time_stamps.battery_status = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_RADIO_STATUS:
		{
#ifdef MAVLINK_DEBUG
			printf("MAVLINK_MSG_ID_RADIO_STATUS\n");
#endif
			mavlink_msg_radio_status_decode(&message,
					&(current_messages.radio_status));
			current_messages.time_stamps.radio_status = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
		{
#ifdef MAVLINK_DEBUG
			printf("MAVLINK_MSG_ID_LOCAL_POSITION_NED\n");
#endif
			mavlink_msg_local_position_ned_decode(&message,
					&(current_messages.local_position_ned));
			current_messages.time_stamps.local_position_ned = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
		{
#ifdef MAVLINK_DEBUG
			printf("MAVLINK_MSG_ID_GLOBAL_POSITION_INT\n");
#endif
			mavlink_msg_global_position_int_decode(&message,
					&(current_messages.global_position_int));
			current_messages.time_stamps.global_position_int = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
		{
#ifdef MAVLINK_DEBUG
			printf("MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED\n");
#endif
			mavlink_msg_position_target_local_ned_decode(&message,
					&(current_messages.position_target_local_ned));
			current_messages.time_stamps.position_target_local_ned =
					get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:
		{
#ifdef MAVLINK_DEBUG
			printf("MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT\n");
#endif
			mavlink_msg_position_target_global_int_decode(&message,
					&(current_messages.position_target_global_int));
			current_messages.time_stamps.position_target_global_int =
					get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_HIGHRES_IMU:
		{
#ifdef MAVLINK_DEBUG
			printf("MAVLINK_MSG_ID_HIGHRES_IMU\n");
#endif
			mavlink_msg_highres_imu_decode(&message,
					&(current_messages.highres_imu));
			current_messages.time_stamps.highres_imu = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_ATTITUDE:
		{
#ifdef MAVLINK_DEBUG
			printf("MAVLINK_MSG_ID_ATTITUDE\n");
#endif
			mavlink_msg_attitude_decode(&message, &(current_messages.attitude));
			current_messages.time_stamps.attitude = get_time_usec();
			break;
		}

		default:
		{
#ifdef MAVLINK_DEBUG
			printf("Warning, did not handle message id %i\n", message.msgid);
#endif
			break;
		}

		} // end: switch msgid


		if(++nMsgHandled >= NUM_MSG_HANDLE)return;
	}

}

bool _MavlinkInterface::readMessage(mavlink_message_t &message)
{
	uint8_t cp;
	mavlink_status_t status;
	uint8_t result;

	while (m_pSerialPort->Read((char*)&cp, 1))
	{
//		if (mavlink_parse_char(MAVLINK_COMM_0, cp, &message, &status))
		result = mavlink_frame_char(MAVLINK_COMM_0, cp, &message, &status);

		if (result == 1)
		{
			//Good message decoded
			last_status = status;
			return true;
		}
		else if(result == 2)
		{
			//Bad CRC
//			printf("ERROR: DROPPED %d PACKETS\n", status.packet_rx_drop_count);
		}

		// check for dropped packets
/*		if ((last_status.packet_rx_drop_count != status.packet_rx_drop_count) &&
				status.packet_rx_drop_count > 0)
		{
			printf("ERROR: DROPPED %d PACKETS\n", status.packet_rx_drop_count);
		}
		last_status = status;
*/
	}

	return false;
}

int _MavlinkInterface::writeMessage(mavlink_message_t message)
{
	char buf[300];

	// Translate message to buffer
	unsigned int len = mavlink_msg_to_send_buffer((uint8_t*) buf, &message);

	// Write buffer to serial port, locks port while writing
	m_pSerialPort->Write(buf, len);

	return len;
}

bool _MavlinkInterface::start(void)
{
	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode << " in MavlinkInterface::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO)<< "MavlinkInterface.start()";

	return true;
}

void _MavlinkInterface::update(void)
{
	while (m_bThreadON)
	{
		//Establish serial connection
		if (m_sportName == "")
		{
			this->sleepThread(1, 0);
			continue;
		}

		//Try to open and setup the serial port
		if (!m_bSerialOpen)
		{
			if (m_pSerialPort->Open((char*)m_sportName.c_str()))
			{
				LOG(INFO)<< "Serial port: "+m_sportName+" connected";
			}
			else
			{
				this->sleepThread(1, 0);
				continue;
			}

			if (!m_pSerialPort->Setup(m_baudRate, 8, 1, false, false))
			{
				LOG(INFO)<< "Serial port: "+m_sportName+" could not be configured";
				m_pSerialPort->Close();
				this->sleepThread(1, 0);
				continue;
			}

			last_status.packet_rx_drop_count = 0;
			m_bSerialOpen = true;
		}


		//Regular update loop
		this->autoFPSfrom();

		//Sending Heartbeat at 1Hz
		sendHeartbeat(USEC_1SEC);

//		requestDataStream(/*MAV_DATA_STREAM_RAW_SENSORS*/MAV_DATA_STREAM_ALL, 1);

//		command_long_doSetMode(MAV_MODE_GUIDED_DISARMED);

		//Handling incoming messages
		handleMessages();

		this->autoFPSto();
	}

}

void _MavlinkInterface::sendHeartbeat(uint64_t interval_usec)
{
	uint64_t timeNow = get_time_usec();

	if(timeNow - m_lastHeartbeat >= interval_usec)
	{
		m_lastHeartbeat = timeNow;

		mavlink_message_t message;
		mavlink_msg_heartbeat_pack(m_systemID, m_componentID, &message, m_type, 0, 0, 0, MAV_STATE_ACTIVE);

		writeMessage(message);

#ifdef MAVLINK_DEBUG
		printf("   SENT HEARTBEAT\n");
#endif
	}

}

void _MavlinkInterface::requestDataStream(uint8_t stream_id, int rate)
{
	mavlink_message_t message;
	mavlink_request_data_stream_t ds;
	ds.target_system = m_systemID;
	ds.target_component = m_targetComponentID;
	ds.req_stream_id = stream_id;
	ds.req_message_rate = rate;
	ds.start_stop = 1;
	mavlink_msg_request_data_stream_encode(m_systemID, m_componentID, &message, &ds);

	writeMessage(message);

#ifdef MAVLINK_DEBUG
	printf("   SENT REQUEST_DATA_STREAM\n");
#endif

	return;
}

void _MavlinkInterface::landing_target(uint8_t stream_id, uint8_t frame, float angle_x, float angle_y, float distance, float size_x, float size_y)
{
	mavlink_message_t message;
	mavlink_landing_target_t ds;

	ds.time_usec = get_time_usec();
	ds.target_num = 0;
	ds.frame = MAV_FRAME_BODY_NED;
	ds.angle_x = angle_x;
	ds.angle_y = angle_y;
	ds.distance = distance;
	ds.size_x = size_x;
	ds.size_y = size_y;
	mavlink_msg_landing_target_encode(m_systemID, m_componentID, &message, &ds);

	writeMessage(message);

#ifdef MAVLINK_DEBUG
	printf("   SENT LANDING_TARGET: ANGLE_X:%f; ANGLE_Y:%f;\n", angle_x, angle_y);
#endif

	return;
}

void _MavlinkInterface::command_long_doSetMode(int mode)
{
	mavlink_message_t message;
	mavlink_command_long_t ds;

	ds.target_system = m_systemID;
	ds.target_component = m_targetComponentID;
	ds.command = MAV_CMD_DO_SET_MODE;
	ds.param1 = mode;
	mavlink_msg_command_long_encode(m_systemID, m_componentID, &message, &ds);

	writeMessage(message);

#ifdef MAVLINK_DEBUG
	printf("   SENT COMMAND_LONG: DO_SET_MODE\n");
#endif

	return;
}



//int _MavlinkInterface::toggleOffboardControl(bool bEnable)
//{
//	if(m_bControlling == bEnable)return -1;
//
//	// Prepare command for off-board mode
//	mavlink_command_long_t com;
//	com.target_system = m_systemID;
//	com.target_component = m_componentID;
//	com.command = MAV_CMD_NAV_GUIDED_ENABLE;
//	com.confirmation = true;
//	com.param1 = (float) bEnable; // flag >0.5 => start, <0.5 => stop
//
//	// Encode
//	mavlink_message_t message;
//	mavlink_msg_command_long_encode(m_systemID, m_componentID, &message, &com);
//
//	// Send the message
//	return writeMessage(message);
//
//}
//
//
///*
// * Set target local ned position
// *
// * Modifies a mavlink_set_position_target_local_ned_t struct with target XYZ locations
// * in the Local NED frame, in meters.
// */
//void _MavlinkInterface::set_position(float x, float y, float z,
//		mavlink_set_position_target_local_ned_t &sp)
//{
//	sp.type_mask =
//	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_POSITION;
//
//	sp.coordinate_frame = MAV_FRAME_LOCAL_NED;
//
//	sp.x = x;
//	sp.y = y;
//	sp.z = z;
//
//	printf("POSITION SETPOINT XYZ = [ %.4f , %.4f , %.4f ] \n", sp.x, sp.y, sp.z);
//
//
//}
//
///*
// * Set target local ned velocity
// *
// * Modifies a mavlink_set_position_target_local_ned_t struct with target VX VY VZ
// * velocities in the Local NED frame, in meters per second.
// */
//void _MavlinkInterface::set_velocity(float vx, float vy, float vz,
//		mavlink_set_position_target_local_ned_t &sp)
//{
//	sp.type_mask =
//	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_VELOCITY;
//
//	sp.coordinate_frame = MAV_FRAME_LOCAL_NED;
//
//	sp.vx = vx;
//	sp.vy = vy;
//	sp.vz = vz;
//
//	//printf("VELOCITY SETPOINT UVW = [ %.4f , %.4f , %.4f ] \n", sp.vx, sp.vy, sp.vz);
//
//}
//
///*
// * Set target local ned acceleration
// *
// * Modifies a mavlink_set_position_target_local_ned_t struct with target AX AY AZ
// * accelerations in the Local NED frame, in meters per second squared.
// */
//void _MavlinkInterface::set_acceleration(float ax, float ay, float az,
//		mavlink_set_position_target_local_ned_t &sp)
//{
//
//	// NOT IMPLEMENTED
//	fprintf(stderr, "set_acceleration doesn't work yet \n");
//	throw 1;
//
//	sp.type_mask =
//	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_ACCELERATION &
//	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_VELOCITY;
//
//	sp.coordinate_frame = MAV_FRAME_LOCAL_NED;
//
//	sp.afx = ax;
//	sp.afy = ay;
//	sp.afz = az;
//}
//
//// the next two need to be called after one of the above
//
///*
// * Set target local ned yaw
// *
// * Modifies a mavlink_set_position_target_local_ned_t struct with a target yaw
// * in the Local NED frame, in radians.
// */
//void _MavlinkInterface::set_yaw(float yaw,
//		mavlink_set_position_target_local_ned_t &sp)
//{
//	sp.type_mask &=
//	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_YAW_ANGLE;
//
//	sp.yaw = yaw;
//
//	printf("POSITION SETPOINT YAW = %.4f \n", sp.yaw);
//
//}
//
///*
// * Set target local ned yaw rate
// *
// * Modifies a mavlink_set_position_target_local_ned_t struct with a target yaw rate
// * in the Local NED frame, in radians per second.
// */
//void _MavlinkInterface::set_yaw_rate(float yaw_rate,
//		mavlink_set_position_target_local_ned_t &sp)
//{
//	sp.type_mask &=
//	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_YAW_RATE;
//
//	sp.yaw_rate = yaw_rate;
//}


}



