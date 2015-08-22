#include "VehicleInterface.h"

VehicleInterface::VehicleInterface()
{
	m_bConnected = false;
}

VehicleInterface::~VehicleInterface()
{
}

bool VehicleInterface::open(char* pSerialName)
{
	m_systemID = 0; // system id
	m_autopilotID = 0; // autopilot component id
	m_companionID = 0; // companion computer component id

//	m_currentMessages.sysid = m_systemID;
//	m_currentMessages.compid = m_autopilotID;

	//Start Serial Port
	m_pSerialPort = new SerialPort();
	if (m_pSerialPort->connect((char*)pSerialName) != true)
	{
		m_bConnected = false;
		return false;
	}

	m_recvMsg.m_cmd = 0;
	m_bConnected = true;

	return true;

	/*
	// --------------------------------------------------------------------------
	//   GET INITIAL POSITION
	// --------------------------------------------------------------------------

	// Wait for initial position ned
	while (!(m_currentMessages.time_stamps.local_position_ned &&
		m_currentMessages.time_stamps.attitude))
	{
	}

	// copy initial position ned
	Mavlink_Messages local_data = m_currentMessages;
	m_initialPosition.x = local_data.local_position_ned.x;
	m_initialPosition.y = local_data.local_position_ned.y;
	m_initialPosition.z = local_data.local_position_ned.z;
	m_initialPosition.vx = local_data.local_position_ned.vx;
	m_initialPosition.vy = local_data.local_position_ned.vy;
	m_initialPosition.vz = local_data.local_position_ned.vz;
	m_initialPosition.yaw = local_data.attitude.yaw;
	m_initialPosition.yaw_rate = local_data.attitude.yawspeed;

	printf("INITIAL POSITION XYZ = [ %.4f , %.4f , %.4f ] \n", m_initialPosition.x, m_initialPosition.y, m_initialPosition.z);
	printf("INITIAL POSITION YAW = %.4f \n", m_initialPosition.yaw);
	printf("\n");
*/
	// we need this before starting the write thread
}

void VehicleInterface::close()
{
//	disable_offboard_control();

	if (m_pSerialPort)
	{
		delete m_pSerialPort;
		m_bConnected = false;
	}
	printf("Serial port closed.\n");
}

// ------------------------------------------------------------------------------
//   Write Message
// ------------------------------------------------------------------------------
/*
bool MavlinkInterface::writeMessage(mavlink_message_t message)
{
	char buf[300];

	// Translate message to buffer
	unsigned len = mavlink_msg_to_send_buffer((uint8_t*)buf, &message);

	return m_pSerialPort->WriteData(buf,len);
}
*/

// ------------------------------------------------------------------------------
//   Read Messages
// ------------------------------------------------------------------------------
bool VehicleInterface::readMessages()
{
/*	mavlink_message_t message;
	mavlink_status_t status;
	char	buf;

	while (m_pSerialPort->ReadData(&buf, 1) > 0)
	{
		// the parsing
		if(mavlink_parse_char(MAVLINK_COMM_1, buf, &message, &status))
		{
			// Report info
			printf("Received message from serial with ID #%d (sys:%d|comp:%d):\n", message.msgid, message.sysid, message.compid);
//			handleMessage(message);
			return true;
		}
	}
*/

	unsigned char	inByte;
	int		byteRead;

	while ((byteRead = m_pSerialPort->ReadData((char*)&inByte,1)) > 0)
	{
		if (m_recvMsg.m_cmd != 0)
		{
			m_recvMsg.m_pBuf[m_recvMsg.m_iByte] = inByte;
			m_recvMsg.m_iByte++;

			if (m_recvMsg.m_iByte == 2)	//Payload length
			{
				m_recvMsg.m_payloadLen = inByte;
			}
			else if (m_recvMsg.m_iByte == m_recvMsg.m_payloadLen + MAVLINK_HEADDER_LEN + 1)
			{
				//decode the command
//				hostCommand();
//				m_recvMsg.m_cmd = 0;

				return true; //Execute one command at a time
			}
		}
		else if (inByte == MAVLINK_BEGIN)//(inByte == VL_CMD_BEGIN || inByte == MAVLINK_BEGIN)
		{
			m_recvMsg.m_cmd = inByte;
			m_recvMsg.m_pBuf[0] = inByte;
			m_recvMsg.m_iByte = 1;
			m_recvMsg.m_payloadLen = 0;
		}
	}

	return false;

}



// ------------------------------------------------------------------------------
//   Write Setpoint Message
// ------------------------------------------------------------------------------
void VehicleInterface::rc_overide(int numChannel, int* pChannels)
{
	/*
	// Prepare command for off-board mode
	mavlink_command_long_t com;
	com.target_system = m_systemID;
	com.target_component = m_autopilotID;
	com.command = MAV_CMD_NAV_GUIDED_ENABLE;
	com.confirmation = true;
//	com.param1 = (float)flag; // flag >0.5 => start, <0.5 => stop

	// Encode
	mavlink_message_t message;
	mavlink_msg_command_long_encode(m_systemID, m_companionID, &message, &com);
	mavlink_msg_to_send_buffer((uint8_t*)m_pBuf, &message);

	// Send the message
	int len = m_pSerialPort->WriteData(m_pBuf,sizeof(message));
	*/

	if (!m_bConnected)return;

	int len;
	int pwm;

	m_pBuf[0] = 0xFE;//Mavlink begin
	m_pBuf[1] = 1 + numChannel * 2;	//Payload Length
	m_pBuf[2] = 0;
	m_pBuf[3] = numChannel;

	for (int i = 0; i < numChannel; i++)
	{
		m_pBuf[4 + i * 2] = (unsigned char)(pChannels[i] & 0xFF);
		m_pBuf[4 + i * 2 + 1] = (unsigned char)(pChannels[i] >> 8);
	}

	len = 4 + numChannel * 2;

	m_pSerialPort->WriteData((char*)m_pBuf, len);

}

void VehicleInterface::controlMode(int mode)
{
	if (!m_bConnected)return;

	m_pBuf[0] = 0xFE;//Mavlink begin
	m_pBuf[1] = 1;
	m_pBuf[2] = 1;
	m_pBuf[3] = (unsigned char)mode;

	m_pSerialPort->WriteData((char*)m_pBuf, 4);
}


/*

void MavlinkInterface::handleMessage(mavlink_message_t message)
{
//	Time_Stamps this_timestamps;

	// Store message sysid and compid.
	// Note this doesn't handle multiple message sources.
	m_currentMessages.sysid = message.sysid;
	m_currentMessages.compid = message.compid;
	m_systemID = m_currentMessages.sysid;
	m_autopilotID = m_currentMessages.compid;


	// Handle Message ID
	switch (message.msgid)
	{
		case MAVLINK_MSG_ID_HEARTBEAT:
		{
			//printf("MAVLINK_MSG_ID_HEARTBEAT\n");
			mavlink_msg_heartbeat_decode(&message, &(m_currentMessages.heartbeat));
			m_currentMessages.time_stamps.heartbeat = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_SYS_STATUS:
		{
			//printf("MAVLINK_MSG_ID_SYS_STATUS\n");
			mavlink_msg_sys_status_decode(&message, &(m_currentMessages.sys_status));
			m_currentMessages.time_stamps.sys_status = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_BATTERY_STATUS:
		{
			//printf("MAVLINK_MSG_ID_BATTERY_STATUS\n");
			mavlink_msg_battery_status_decode(&message, &(m_currentMessages.battery_status));
			m_currentMessages.time_stamps.battery_status = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_RADIO_STATUS:
		{
			//printf("MAVLINK_MSG_ID_RADIO_STATUS\n");
			mavlink_msg_radio_status_decode(&message, &(m_currentMessages.radio_status));
			m_currentMessages.time_stamps.radio_status = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
		{
			//printf("MAVLINK_MSG_ID_LOCAL_POSITION_NED\n");
			mavlink_msg_local_position_ned_decode(&message, &(m_currentMessages.local_position_ned));
			m_currentMessages.time_stamps.local_position_ned = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
		{
			//printf("MAVLINK_MSG_ID_GLOBAL_POSITION_INT\n");
			mavlink_msg_global_position_int_decode(&message, &(m_currentMessages.global_position_int));
			m_currentMessages.time_stamps.global_position_int = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
		{
			//printf("MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED\n");
			mavlink_msg_position_target_local_ned_decode(&message, &(m_currentMessages.position_target_local_ned));
			m_currentMessages.time_stamps.position_target_local_ned = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:
		{
			//printf("MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT\n");
			mavlink_msg_position_target_global_int_decode(&message, &(m_currentMessages.position_target_global_int));
			m_currentMessages.time_stamps.position_target_global_int = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_HIGHRES_IMU:
		{
			//printf("MAVLINK_MSG_ID_HIGHRES_IMU\n");
			mavlink_msg_highres_imu_decode(&message, &(m_currentMessages.highres_imu));
			m_currentMessages.time_stamps.highres_imu = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_ATTITUDE:
		{
			//printf("MAVLINK_MSG_ID_ATTITUDE\n");
			mavlink_msg_attitude_decode(&message, &(m_currentMessages.attitude));
			m_currentMessages.time_stamps.attitude = get_time_usec();
			break;
		}

		default:
		{
			// printf("Warning, did not handle message id %i\n",message.msgid);
			break;
		}

	} // end: if read message

}



*/










/*

// ------------------------------------------------------------------------------
//   Update Setpoint
// ------------------------------------------------------------------------------
void MavlinkInterface::update_setpoint(mavlink_set_position_target_local_ned_t setpoint)
{
	current_setpoint = setpoint;
}


// ----------------------------------------------------------------------------------
//   Time
// ------------------- ---------------------------------------------------------------
uint64_t get_time_usec()
{
	
//	static struct timeval _time_stamp;
//	gettimeofday(&_time_stamp, NULL);
//	return _time_stamp.tv_sec * 1000000 + _time_stamp.tv_usec;
	
	return 0;
}


// ------------------------------------------------------------------------------
//   Stop Off-Board Mode
// ------------------------------------------------------------------------------
void MavlinkInterface::disable_offboard_control()
{

	// Should only send this command once
	if (m_controlStatus == true)
	{
		printf("DISABLE OFFBOARD MODE\n");

		// ----------------------------------------------------------------------
		//   TOGGLE OFF-BOARD MODE
		// ----------------------------------------------------------------------

		// Sends the command to stop off-board
		int success = toggle_offboard_control(false);

		// Check the command was written
		if (success)
			m_controlStatus = false;
		else
		{
			fprintf(stderr, "Error: off-board mode not set, could not write message\n");
			//throw EXIT_FAILURE;
		}

		printf("\n");

	} // end: if offboard_status

}


// ------------------------------------------------------------------------------
//   Toggle Off-Board Mode
// ------------------------------------------------------------------------------
int MavlinkInterface::toggle_offboard_control(bool flag)
{
	// Prepare command for off-board mode
	mavlink_command_long_t com;
	com.target_system = m_systemID;
	com.target_component = m_autopilotID;
	com.command = MAV_CMD_NAV_GUIDED_ENABLE;
	com.confirmation = true;
	com.param1 = (float)flag; // flag >0.5 => start, <0.5 => stop

	// Encode
	mavlink_message_t message;
	mavlink_msg_command_long_encode(m_systemID, m_companionID, &message, &com);

	// Send the message
	//!!	int len = m_pSerialPort->writeMessage(message);

	// Done!
	return 0;// len;
}

// ------------------------------------------------------------------------------
//   Write Setpoint Message
// ------------------------------------------------------------------------------
void MavlinkInterface::write_setpoint()
{
	// --------------------------------------------------------------------------
	//   PACK PAYLOAD
	// --------------------------------------------------------------------------

	// pull from position target
	mavlink_set_position_target_local_ned_t sp = current_setpoint;

	// double check some system parameters
	if (!sp.time_boot_ms)
		sp.time_boot_ms = (uint32_t)(get_time_usec() / 1000);
	sp.target_system = m_systemID;
	sp.target_component = m_autopilotID;


	// --------------------------------------------------------------------------
	//   ENCODE
	// --------------------------------------------------------------------------

	mavlink_message_t message;
	mavlink_msg_set_position_target_local_ned_encode(m_systemID, m_companionID, &message, &sp);


	// --------------------------------------------------------------------------
	//   WRITE
	// --------------------------------------------------------------------------

	// do the write
	int len = writeMessage(message);

	// check the write
	if (!len > 0)
		fprintf(stderr, "WARNING: could not send POSITION_TARGET_LOCAL_NED \n");
	//	else
	//		printf("%lu POSITION_TARGET  = [ %f , %f , %f ] \n", m_writeCount, position_target.x, position_target.y, position_target.z);

	return;
}


// ------------------------------------------------------------------------------
//   Start Off-Board Mode
// ------------------------------------------------------------------------------
void MavlinkInterface::enable_offboard_control()
{
	// Should only send this command once
	if (m_controlStatus == false)
	{
		printf("ENABLE OFFBOARD MODE\n");

		// ----------------------------------------------------------------------
		//   TOGGLE OFF-BOARD MODE
		// ----------------------------------------------------------------------

		// Sends the command to go off-board
		int success = toggle_offboard_control(true);

		// Check the command was written
		if (success)
			m_controlStatus = true;
		else
		{
			fprintf(stderr, "Error: off-board mode not set, could not write message\n");
			//throw EXIT_FAILURE;
		}

		printf("\n");

	} // end: if not offboard_status

}




*/