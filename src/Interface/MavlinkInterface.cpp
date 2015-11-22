#include "MavlinkInterface.h"
#include "../Utility/util.h"

namespace kai
{

MavlinkInterface::MavlinkInterface()
{
	m_bSerialConnected = false;
	m_sportName = "";
	m_bThreadON = false;
	m_pSerialPort = NULL;
	m_baudRate = 57600;

	// whether the autopilot is in offboard control mode
	m_bControlling = false;

	system_id = 0;
	autopilot_id = 0;
	companion_id = 0;

	current_messages.sysid = system_id;
	current_messages.compid = autopilot_id;

	current_messages.attitude.pitch = 0;
	current_messages.attitude.roll = 0;
	current_messages.attitude.yaw = 0;
}

MavlinkInterface::~MavlinkInterface()
{
	if (m_pSerialPort)
	{
		delete m_pSerialPort;
	}
}

void MavlinkInterface::setSerial(string name, int baudrate)
{
	m_sportName = name;
	m_baudRate = baudrate;
}

bool MavlinkInterface::open(void)
{
	system_id = 0;
	autopilot_id = 0;
	companion_id = 0;

	current_messages.sysid = system_id;
	current_messages.compid = autopilot_id;

	//Start Serial Port
	m_pSerialPort = new SerialPort();
	if (m_pSerialPort->Open((char*) m_sportName.c_str()) != true)
	{
		m_bSerialConnected = false;
		return false;
	}

	if (!m_pSerialPort->Setup(m_baudRate, 8, 1, false, false))
	{
		printf("failure, could not configure port.\n");
		return false;
	}

	last_status.packet_rx_drop_count = 0;

	m_bSerialConnected = true;
	return true;

}

void MavlinkInterface::close()
{
//	disable_offboard_control();

	if (m_pSerialPort)
	{
		delete m_pSerialPort;
		m_bSerialConnected = false;
	}
	printf("Serial port closed.\n");
}

void MavlinkInterface::handleMessages()
{
	Time_Stamps this_timestamps;
	mavlink_message_t message;
	int nMsgHandled;

	nMsgHandled = 0;

	//Handle Message while new message is received
	while (readMessage(message))
	{
		// Note this doesn't handle multiple message sources.
		current_messages.sysid = message.sysid;
		current_messages.compid = message.compid;

		system_id = current_messages.sysid;
		autopilot_id = current_messages.compid;

		// Handle Message ID
		switch (message.msgid)
		{

		case MAVLINK_MSG_ID_HEARTBEAT:
		{
//			printf("MAVLINK_MSG_ID_HEARTBEAT\n");
			mavlink_msg_heartbeat_decode(&message,
					&(current_messages.heartbeat));
			current_messages.time_stamps.heartbeat = get_time_usec();
			this_timestamps.heartbeat = current_messages.time_stamps.heartbeat;
			break;
		}

		case MAVLINK_MSG_ID_SYS_STATUS:
		{
//			printf("MAVLINK_MSG_ID_SYS_STATUS\n");
			mavlink_msg_sys_status_decode(&message,
					&(current_messages.sys_status));
			current_messages.time_stamps.sys_status = get_time_usec();
			this_timestamps.sys_status =
					current_messages.time_stamps.sys_status;
			break;
		}

		case MAVLINK_MSG_ID_BATTERY_STATUS:
		{
//			printf("MAVLINK_MSG_ID_BATTERY_STATUS\n");
			mavlink_msg_battery_status_decode(&message,
					&(current_messages.battery_status));
			current_messages.time_stamps.battery_status = get_time_usec();
			this_timestamps.battery_status =
					current_messages.time_stamps.battery_status;
			break;
		}

		case MAVLINK_MSG_ID_RADIO_STATUS:
		{
//			printf("MAVLINK_MSG_ID_RADIO_STATUS\n");
			mavlink_msg_radio_status_decode(&message,
					&(current_messages.radio_status));
			current_messages.time_stamps.radio_status = get_time_usec();
			this_timestamps.radio_status =
					current_messages.time_stamps.radio_status;
			break;
		}

		case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
		{
//			printf("MAVLINK_MSG_ID_LOCAL_POSITION_NED\n");
			mavlink_msg_local_position_ned_decode(&message,
					&(current_messages.local_position_ned));
			current_messages.time_stamps.local_position_ned = get_time_usec();
			this_timestamps.local_position_ned =
					current_messages.time_stamps.local_position_ned;
			break;
		}

		case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
		{
//			printf("MAVLINK_MSG_ID_GLOBAL_POSITION_INT\n");
			mavlink_msg_global_position_int_decode(&message,
					&(current_messages.global_position_int));
			current_messages.time_stamps.global_position_int = get_time_usec();
			this_timestamps.global_position_int =
					current_messages.time_stamps.global_position_int;
			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
		{
//			printf("MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED\n");
			mavlink_msg_position_target_local_ned_decode(&message,
					&(current_messages.position_target_local_ned));
			current_messages.time_stamps.position_target_local_ned =
					get_time_usec();
			this_timestamps.position_target_local_ned =
					current_messages.time_stamps.position_target_local_ned;
			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:
		{
//			printf("MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT\n");
			mavlink_msg_position_target_global_int_decode(&message,
					&(current_messages.position_target_global_int));
			current_messages.time_stamps.position_target_global_int =
					get_time_usec();
			this_timestamps.position_target_global_int =
					current_messages.time_stamps.position_target_global_int;
			break;
		}

		case MAVLINK_MSG_ID_HIGHRES_IMU:
		{
//			printf("MAVLINK_MSG_ID_HIGHRES_IMU\n");
			mavlink_msg_highres_imu_decode(&message,
					&(current_messages.highres_imu));
			current_messages.time_stamps.highres_imu = get_time_usec();
			this_timestamps.highres_imu =
					current_messages.time_stamps.highres_imu;
			break;
		}

		case MAVLINK_MSG_ID_ATTITUDE:
		{
//			printf("MAVLINK_MSG_ID_ATTITUDE\n");
			mavlink_msg_attitude_decode(&message, &(current_messages.attitude));
			current_messages.time_stamps.attitude = get_time_usec();
			this_timestamps.attitude = current_messages.time_stamps.attitude;
			break;
		}

		default:
		{
//			printf("Warning, did not handle message id %i\n", message.msgid);
			break;
		}

		} // end: switch msgid


		if(++nMsgHandled >= NUM_MSG_HANDLE)return;
	}

}

bool MavlinkInterface::readMessage(mavlink_message_t &message)
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

int MavlinkInterface::writeMessage(mavlink_message_t message)
{
	char buf[300];

	// Translate message to buffer
	unsigned int len = mavlink_msg_to_send_buffer((uint8_t*) buf, &message);

	// Write buffer to serial port, locks port while writing
	m_pSerialPort->Write(buf, len);

	return len;
}

bool MavlinkInterface::start(void)
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

void MavlinkInterface::update(void)
{
	int tThreadBegin;
	m_tSleep = TRD_INTERVAL_MI_USEC;

	while (m_bThreadON)
	{
		tThreadBegin = time(NULL);

		if (m_sportName == "")
		{
			this->sleepThread(0, m_tSleep);
			continue;
		}

		if (!m_bSerialConnected)
		{
			if (m_pSerialPort->Open((char*)m_sportName.c_str()))
			{
				LOG(INFO)<< "Serial port: "+m_sportName+" connected";
			}
			else
			{
				this->sleepThread(0,m_tSleep);
				continue;
			}
		}

		//Connected to Vehicle
		requestDataStream(MAV_DATA_STREAM_RAW_SENSORS/*MAV_DATA_STREAM_ALL*/, 30);

		handleMessages();







		//send local ned control at 2Hz

		if (m_tSleep > 0)
		{
			//sleepThread can be woke up by this->wakeupThread()
			this->sleepThread(0, m_tSleep);
		}
	}

}

bool MavlinkInterface::complete(void)
{

	return true;
}

void MavlinkInterface::stop(void)
{
	m_bThreadON = false;
	this->wakeupThread();
	pthread_join(m_threadID, NULL);

	LOG(INFO)<< "MavlinkInterface.stop()";
}

void MavlinkInterface::waitForComplete(void)
{
	pthread_join(m_threadID, NULL);
}


void MavlinkInterface::requestDataStream(uint8_t stream_id, int rate)
{
	mavlink_message_t message;
	mavlink_request_data_stream_t ds;
	ds.target_system = system_id;
	ds.target_component = autopilot_id;
	ds.req_stream_id = stream_id;
	ds.req_message_rate = rate;
	ds.start_stop = 1;
	mavlink_msg_request_data_stream_encode(system_id, companion_id, &message,&ds);

	writeMessage(message);

	return;
}

int MavlinkInterface::toggleOffboardControl(bool bEnable)
{
	if(m_bControlling == bEnable)return -1;

	// Prepare command for off-board mode
	mavlink_command_long_t com;
	com.target_system = system_id;
	com.target_component = autopilot_id;
	com.command = MAV_CMD_NAV_GUIDED_ENABLE;
	com.confirmation = true;
	com.param1 = (float) bEnable; // flag >0.5 => start, <0.5 => stop

	// Encode
	mavlink_message_t message;
	mavlink_msg_command_long_encode(system_id, companion_id, &message, &com);

	// Send the message
	return writeMessage(message);

}






/*
 * Set target local ned position
 *
 * Modifies a mavlink_set_position_target_local_ned_t struct with target XYZ locations
 * in the Local NED frame, in meters.
 */
void MavlinkInterface::set_position(float x, float y, float z,
		mavlink_set_position_target_local_ned_t &sp)
{
	sp.type_mask =
	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_POSITION;

	sp.coordinate_frame = MAV_FRAME_LOCAL_NED;

	sp.x = x;
	sp.y = y;
	sp.z = z;

	printf("POSITION SETPOINT XYZ = [ %.4f , %.4f , %.4f ] \n", sp.x, sp.y,
			sp.z);

}

/*
 * Set target local ned velocity
 *
 * Modifies a mavlink_set_position_target_local_ned_t struct with target VX VY VZ
 * velocities in the Local NED frame, in meters per second.
 */
void MavlinkInterface::set_velocity(float vx, float vy, float vz,
		mavlink_set_position_target_local_ned_t &sp)
{
	sp.type_mask =
	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_VELOCITY;

	sp.coordinate_frame = MAV_FRAME_LOCAL_NED;

	sp.vx = vx;
	sp.vy = vy;
	sp.vz = vz;

	//printf("VELOCITY SETPOINT UVW = [ %.4f , %.4f , %.4f ] \n", sp.vx, sp.vy, sp.vz);

}

/*
 * Set target local ned acceleration
 *
 * Modifies a mavlink_set_position_target_local_ned_t struct with target AX AY AZ
 * accelerations in the Local NED frame, in meters per second squared.
 */
void MavlinkInterface::set_acceleration(float ax, float ay, float az,
		mavlink_set_position_target_local_ned_t &sp)
{

	// NOT IMPLEMENTED
	fprintf(stderr, "set_acceleration doesn't work yet \n");
	throw 1;

	sp.type_mask =
	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_ACCELERATION &
	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_VELOCITY;

	sp.coordinate_frame = MAV_FRAME_LOCAL_NED;

	sp.afx = ax;
	sp.afy = ay;
	sp.afz = az;
}

// the next two need to be called after one of the above

/*
 * Set target local ned yaw
 *
 * Modifies a mavlink_set_position_target_local_ned_t struct with a target yaw
 * in the Local NED frame, in radians.
 */
void MavlinkInterface::set_yaw(float yaw,
		mavlink_set_position_target_local_ned_t &sp)
{
	sp.type_mask &=
	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_YAW_ANGLE;

	sp.yaw = yaw;

	printf("POSITION SETPOINT YAW = %.4f \n", sp.yaw);

}

/*
 * Set target local ned yaw rate
 *
 * Modifies a mavlink_set_position_target_local_ned_t struct with a target yaw rate
 * in the Local NED frame, in radians per second.
 */
void MavlinkInterface::set_yaw_rate(float yaw_rate,
		mavlink_set_position_target_local_ned_t &sp)
{
	sp.type_mask &=
	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_YAW_RATE;

	sp.yaw_rate = yaw_rate;
}


}

