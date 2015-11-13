#include "MavlinkInterface.h"

namespace kai
{

MavlinkInterface::MavlinkInterface()
{
	m_bConnected = false;
	m_sportName = "";
	m_bThreadON = false;
	m_pSerialPort = NULL;


	// initialize attributes
	write_count = 0;
	control_status = 0;     // whether the autopilot is in offboard control mode

	system_id = 0;
	autopilot_id = 0;
	companion_id = 0;

	current_messages.sysid = system_id;
	current_messages.compid = autopilot_id;

	serial_port = serial_port_; // serial port management object


}

MavlinkInterface::~MavlinkInterface()
{
}

void MavlinkInterface::setSerialName(string name)
{
	m_sportName = name;
}

bool MavlinkInterface::open(void)
{
	system_id = 0;
	autopilot_id = 0;
	companion_id = 0;

	currentMessages.sysid = m_systemID;
	currentMessages.compid = m_autopilotID;

	//Start Serial Port
	m_pSerialPort = new SerialPort();
	if (m_pSerialPort->connect((char*)m_sportName.c_str()) != true)
	{
		m_bConnected = false;
		return false;
	}

	m_bConnected = true;






	while (not current_messages.sysid)
	{
		if (time_to_exit)
			return;
		usleep(500000); // check at 2Hz
	}

	printf("Found\n");

	// now we know autopilot is sending messages
	printf("\n");

	// --------------------------------------------------------------------------
	//   GET SYSTEM and COMPONENT IDs
	// --------------------------------------------------------------------------

	// This comes from the heartbeat, which in theory should only come from
	// the autopilot we're directly connected to it.  If there is more than one
	// vehicle then we can't expect to discover id's like this.
	// In which case set the id's manually.

	// System ID
	if (not system_id)
	{
		system_id = current_messages.sysid;
		printf("GOT VEHICLE SYSTEM ID: %i\n", system_id);
	}

	// Component ID
	if (not autopilot_id)
	{
		autopilot_id = current_messages.compid;
		printf("GOT AUTOPILOT COMPONENT ID: %i\n", autopilot_id);
		printf("\n");
	}

	// --------------------------------------------------------------------------
	//   GET INITIAL POSITION
	// --------------------------------------------------------------------------

	// Wait for initial position ned
	/*
	while (not (current_messages.time_stamps.local_position_ned
			&& current_messages.time_stamps.attitude))
	{
		if (time_to_exit)
			return;
		usleep(500000);
	}
*/
	//todo

	// copy initial position ned
	Mavlink_Messages local_data = current_messages;
	initial_position.x = local_data.local_position_ned.x;
	initial_position.y = local_data.local_position_ned.y;
	initial_position.z = local_data.local_position_ned.z;
	initial_position.vx = local_data.local_position_ned.vx;
	initial_position.vy = local_data.local_position_ned.vy;
	initial_position.vz = local_data.local_position_ned.vz;
	initial_position.yaw = local_data.attitude.yaw;
	initial_position.yaw_rate = local_data.attitude.yawspeed;

	printf("INITIAL POSITION XYZ = [ %.4f , %.4f , %.4f ] \n",
			initial_position.x, initial_position.y, initial_position.z);
	printf("INITIAL POSITION YAW = %.4f \n", initial_position.yaw);
	printf("\n");

	// we need this before starting the write thread

	// --------------------------------------------------------------------------
	//   WRITE THREAD
	// --------------------------------------------------------------------------
	printf("START WRITE THREAD \n");

	result = pthread_create(&write_tid, NULL,
			&start_autopilot_interface_write_thread, this);
	if (result)
		throw result;

	// wait for it to be started
	while (not writing_status)
		usleep(100000); // 10Hz

	// now we're streaming setpoint commands
	printf("\n");








	return true;


}

void MavlinkInterface::close()
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
//   Read Messages
// ------------------------------------------------------------------------------
bool MavlinkInterface::readMessages()
{
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
				m_recvMsg.m_cmd = 0;

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



bool MavlinkInterface::start(void)
{
	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode << " in MavlinkInterface::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO) << "MavlinkInterface.start()";

	return true;
}

void MavlinkInterface::update(void)
{
	int tThreadBegin;
	m_tSleep = TRD_INTERVAL_MI_USEC;

	while (m_bThreadON)
	{
		tThreadBegin = time(NULL);



/*
		if (g_pVehicle->open((char*)g_serialPort.c_str()) != true)
		{
			LOG(ERROR)<< "Cannot open serial port, Working in CV mode only";
		}
		*/



		if(m_tSleep>0)
		{
			//sleepThread can be woke up by this->wakeupThread()
			this->sleepThread(0,m_tSleep);
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

	LOG(INFO) << "MavlinkInterface.stop()";
}

void MavlinkInterface::waitForComplete(void)
{
	pthread_join(m_threadID, NULL);
}














// ------------------------------------------------------------------------------
//   Start Off-Board Mode
// ------------------------------------------------------------------------------
void MavlinkInterface::enable_offboard_control()
{
	// Should only send this command once
	if (control_status == false)
	{
		printf("ENABLE OFFBOARD MODE\n");

		// ----------------------------------------------------------------------
		//   TOGGLE OFF-BOARD MODE
		// ----------------------------------------------------------------------

		// Sends the command to go off-board
		int success = toggle_offboard_control(true);

		// Check the command was written
		if (success)
			control_status = true;
		else
		{
			fprintf(stderr,
					"Error: off-board mode not set, could not write message\n");
			//throw EXIT_FAILURE;
		}

		printf("\n");

	} // end: if not offboard_status

}

// ------------------------------------------------------------------------------
//   Stop Off-Board Mode
// ------------------------------------------------------------------------------
void MavlinkInterface::disable_offboard_control()
{

	// Should only send this command once
	if (control_status == true)
	{
		printf("DISABLE OFFBOARD MODE\n");

		// ----------------------------------------------------------------------
		//   TOGGLE OFF-BOARD MODE
		// ----------------------------------------------------------------------

		// Sends the command to stop off-board
		int success = toggle_offboard_control(false);

		// Check the command was written
		if (success)
			control_status = false;
		else
		{
			fprintf(stderr,
					"Error: off-board mode not set, could not write message\n");
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
	com.target_system = system_id;
	com.target_component = autopilot_id;
	com.command = MAV_CMD_NAV_GUIDED_ENABLE;
	com.confirmation = true;
	com.param1 = (float) flag; // flag >0.5 => start, <0.5 => stop

	// Encode
	mavlink_message_t message;
	mavlink_msg_command_long_encode(system_id, companion_id, &message, &com);

	// Send the message
	int len = serial_port->write_message(message);




	// Done!
	return len;
}










// ----------------------------------------------------------------------------------
//   Setpoint Helper Functions
// ----------------------------------------------------------------------------------

// choose one of the next three

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
void MavlinkInterface::set_yaw(float yaw, mavlink_set_position_target_local_ned_t &sp)
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
void MavlinkInterface::set_yaw_rate(float yaw_rate, mavlink_set_position_target_local_ned_t &sp)
{
	sp.type_mask &=
	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_YAW_RATE;

	sp.yaw_rate = yaw_rate;
}





// ------------------------------------------------------------------------------
//   Read Messages
// ------------------------------------------------------------------------------
void MavlinkInterface::read_messages()
{
	bool success;               // receive success flag
	bool received_all = false;  // receive only one message
	Time_Stamps this_timestamps;

	// Blocking wait for new data
	while (not received_all and not time_to_exit)
	{
		// ----------------------------------------------------------------------
		//   READ MESSAGE
		// ----------------------------------------------------------------------
		mavlink_message_t message;
		success = serial_port->read_message(message);

		// ----------------------------------------------------------------------
		//   HANDLE MESSAGE
		// ----------------------------------------------------------------------
		if (success)
		{

			// Store message sysid and compid.
			// Note this doesn't handle multiple message sources.
			current_messages.sysid = message.sysid;
			current_messages.compid = message.compid;

			// Handle Message ID
			switch (message.msgid)
			{

			case MAVLINK_MSG_ID_HEARTBEAT:
			{
				printf("MAVLINK_MSG_ID_HEARTBEAT\n");
				mavlink_msg_heartbeat_decode(&message,
						&(current_messages.heartbeat));
				current_messages.time_stamps.heartbeat = get_time_usec();
				this_timestamps.heartbeat =
						current_messages.time_stamps.heartbeat;
				break;
			}

			case MAVLINK_MSG_ID_SYS_STATUS:
			{
				printf("MAVLINK_MSG_ID_SYS_STATUS\n");
				mavlink_msg_sys_status_decode(&message,
						&(current_messages.sys_status));
				current_messages.time_stamps.sys_status = get_time_usec();
				this_timestamps.sys_status =
						current_messages.time_stamps.sys_status;
				break;
			}

			case MAVLINK_MSG_ID_BATTERY_STATUS:
			{
				printf("MAVLINK_MSG_ID_BATTERY_STATUS\n");
				mavlink_msg_battery_status_decode(&message,
						&(current_messages.battery_status));
				current_messages.time_stamps.battery_status = get_time_usec();
				this_timestamps.battery_status =
						current_messages.time_stamps.battery_status;
				break;
			}

			case MAVLINK_MSG_ID_RADIO_STATUS:
			{
				printf("MAVLINK_MSG_ID_RADIO_STATUS\n");
				mavlink_msg_radio_status_decode(&message,
						&(current_messages.radio_status));
				current_messages.time_stamps.radio_status = get_time_usec();
				this_timestamps.radio_status =
						current_messages.time_stamps.radio_status;
				break;
			}

			case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
			{
				printf("MAVLINK_MSG_ID_LOCAL_POSITION_NED\n");
				mavlink_msg_local_position_ned_decode(&message,
						&(current_messages.local_position_ned));
				current_messages.time_stamps.local_position_ned =
						get_time_usec();
				this_timestamps.local_position_ned =
						current_messages.time_stamps.local_position_ned;
				break;
			}

			case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
			{
				printf("MAVLINK_MSG_ID_GLOBAL_POSITION_INT\n");
				mavlink_msg_global_position_int_decode(&message,
						&(current_messages.global_position_int));
				current_messages.time_stamps.global_position_int =
						get_time_usec();
				this_timestamps.global_position_int =
						current_messages.time_stamps.global_position_int;
				break;
			}

			case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
			{
				printf("MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED\n");
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
				printf("MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT\n");
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
				printf("MAVLINK_MSG_ID_HIGHRES_IMU\n");
				mavlink_msg_highres_imu_decode(&message,
						&(current_messages.highres_imu));
				current_messages.time_stamps.highres_imu = get_time_usec();
				this_timestamps.highres_imu =
						current_messages.time_stamps.highres_imu;
				break;
			}

			case MAVLINK_MSG_ID_ATTITUDE:
			{
				printf("MAVLINK_MSG_ID_ATTITUDE\n");
				mavlink_msg_attitude_decode(&message,
						&(current_messages.attitude));
				current_messages.time_stamps.attitude = get_time_usec();
				this_timestamps.attitude =
						current_messages.time_stamps.attitude;
				break;
			}

			default:
			{
				 printf("Warning, did not handle message id %i\n",message.msgid);
				break;
			}

			} // end: switch msgid

		} // end: if read message

		// Check for receipt of all items
/*		received_all = this_timestamps.heartbeat && this_timestamps.sys_status
				&&
//				this_timestamps.battery_status             &&
//				this_timestamps.radio_status               &&
				this_timestamps.local_position_ned
				&&
//				this_timestamps.global_position_int        &&
//				this_timestamps.position_target_local_ned  &&
				this_timestamps.position_target_global_int
				&& this_timestamps.highres_imu && this_timestamps.attitude;
*/
		received_all = this_timestamps.heartbeat;
//				&& this_timestamps.sys_status
//				&&
//				this_timestamps.battery_status             &&
//				this_timestamps.radio_status               &&
//				this_timestamps.local_position_ned
//				&&
//				this_timestamps.global_position_int        &&
//				this_timestamps.position_target_local_ned  &&
//				this_timestamps.position_target_global_int
//				&& this_timestamps.highres_imu && this_timestamps.attitude;



		// give the write thread time to use the port
		if (writing_status > false)
			usleep(100); // look for components of batches at 10kHz

	} // end: while not received all

	return;
}

// ------------------------------------------------------------------------------
//   Write Message
// ------------------------------------------------------------------------------
int MavlinkInterface::write_message(mavlink_message_t message)
{
	// do the write
	int len = serial_port->write_message(message);

	// book keep
	write_count++;

	// Done!
	return len;
}





// ------------------------------------------------------------------------------
//   Read Thread
// ------------------------------------------------------------------------------
void MavlinkInterface::read_thread()
{
	reading_status = true;

	while (not time_to_exit)
	{
		read_messages();
		usleep(100000); // Read batches at 10Hz
	}

	reading_status = false;

	return;
}


// ------------------------------------------------------------------------------
//   Write Thread
// ------------------------------------------------------------------------------
void MavlinkInterface::write_thread(void)
{
	// signal startup
	writing_status = 2;

	// prepare an initial setpoint, just stay put
	mavlink_set_position_target_local_ned_t sp;
	sp.type_mask = MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_VELOCITY &
	MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_YAW_RATE;
	sp.coordinate_frame = MAV_FRAME_LOCAL_NED;
	sp.vx = 0.0;
	sp.vy = 0.0;
	sp.vz = 0.0;
	sp.yaw_rate = 0.0;

	// set position target
	current_setpoint = sp;

	// write a message and signal writing
	write_setpoint();
	writing_status = true;

	// Pixhawk needs to see off-board commands at minimum 2Hz,
	// otherwise it will go into fail safe
	while (not time_to_exit)
	{
		usleep(250000);   // Stream at 4Hz
		write_setpoint();
	}

	// signal end
	writing_status = false;

	return;

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
	if (not sp.time_boot_ms)
		sp.time_boot_ms = (uint32_t) (get_time_usec() / 1000);
	sp.target_system = system_id;
	sp.target_component = autopilot_id;

	// --------------------------------------------------------------------------
	//   ENCODE
	// --------------------------------------------------------------------------

	mavlink_message_t message;
	mavlink_msg_set_position_target_local_ned_encode(system_id, companion_id,
			&message, &sp);

	// --------------------------------------------------------------------------
	//   WRITE
	// --------------------------------------------------------------------------

	// do the write
	int len = write_message(message);

	// check the write
	if (not len > 0)
		fprintf(stderr, "WARNING: could not send POSITION_TARGET_LOCAL_NED \n");
	//	else
	//		printf("%lu POSITION_TARGET  = [ %f , %f , %f ] \n", write_count, position_target.x, position_target.y, position_target.z);







	//test
	mavlink_request_data_stream_t ds;
	ds.target_system = system_id;
	ds.target_component = autopilot_id;
	ds.req_stream_id = MAV_DATA_STREAM_ALL;//MAV_DATA_STREAM_POSITION;
	ds.req_message_rate = 1;
	ds.start_stop = 1;
	mavlink_msg_request_data_stream_encode(system_id,companion_id,&message,&ds);

	len = write_message(message);

	// check the write
	if (not len > 0)
		fprintf(stderr, "WARNING: could not send request data stream \n");





	return;
}






}

