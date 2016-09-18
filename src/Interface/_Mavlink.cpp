#include "../Utility/util.h"
#include "_Mavlink.h"

namespace kai
{

_Mavlink::_Mavlink()
{
	_ThreadBase();

	m_sportName = "";
	m_pSerialPort = NULL;
	m_baudRate = 115200;
	m_systemID = 1;
	m_componentID = MAV_COMP_ID_PATHPLANNER;
	m_type = MAV_TYPE_ONBOARD_CONTROLLER;
	m_targetComponentID = 0;

	current_messages.attitude.pitch = 0;
	current_messages.attitude.roll = 0;
	current_messages.attitude.yaw = 0;
	current_messages.attitude.pitchspeed = 0;
	current_messages.attitude.rollspeed = 0;
	current_messages.attitude.yawspeed = 0;

}

_Mavlink::~_Mavlink()
{
	if (m_pSerialPort)
	{
		delete m_pSerialPort;
	}
}

bool _Mavlink::init(Config* pConfig)
{
	if (this->_ThreadBase::init(pConfig)==false)
		return false;

	F_ERROR_F(pConfig->v("portName", &m_sportName));
	F_ERROR_F(pConfig->v("baudrate", &m_baudRate));

	m_systemID = 1;
	m_componentID = MAV_COMP_ID_PATHPLANNER;
	m_type = MAV_TYPE_ONBOARD_CONTROLLER;
	m_targetComponentID = 0;

	current_messages.sysid = 0;
	current_messages.compid = 0;

	last_status.packet_rx_drop_count = 0;

	//Start Serial Port
	m_pSerialPort = new SerialPort();

	return true;
}

void _Mavlink::close()
{
	if (m_pSerialPort)
	{
		delete m_pSerialPort;
		m_pSerialPort = NULL;
	}
	printf("Serial port closed.\n");
}

void _Mavlink::handleMessages()
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

bool _Mavlink::readMessage(mavlink_message_t &message)
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

int _Mavlink::writeMessage(mavlink_message_t message)
{
	char buf[300];

	// Translate message to buffer
	unsigned int len = mavlink_msg_to_send_buffer((uint8_t*) buf, &message);

	// Write buffer to serial port, locks port while writing
	m_pSerialPort->Write(buf, len);

	return len;
}

bool _Mavlink::start(void)
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

void _Mavlink::update(void)
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
		if (!m_pSerialPort->IsConnected())
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
		}

		//Regular update loop
		this->autoFPSfrom();

		//Handling incoming messages
		handleMessages();

		this->autoFPSto();
	}

}

void _Mavlink::sendHeartbeat(void)
{
	mavlink_message_t message;
	mavlink_msg_heartbeat_pack(m_systemID, m_componentID, &message, m_type, 0, 0, 0, MAV_STATE_ACTIVE);

	writeMessage(message);
}

void _Mavlink::requestDataStream(uint8_t stream_id, int rate)
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

void _Mavlink::landing_target(uint8_t stream_id, uint8_t frame, float angle_x, float angle_y, float distance, float size_x, float size_y)
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

void _Mavlink::command_long_doSetMode(int mode)
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


#define PUTTEXT(x,y,t) cv::putText(*pMat, String(t),Point(x, y),FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1)

bool _Mavlink::draw(Frame* pFrame, iVec4* pTextPos)
{
	if (pFrame == NULL)
		return false;

	Mat* pMat = pFrame->getCMat();

	char strBuf[512];
	std::string strInfo;

	if (m_pSerialPort->IsConnected())
	{
		putText(*pFrame->getCMat(), "Mavlink FPS: " + i2str(getFrameRate()),
				cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	}
	else
	{
		putText(*pFrame->getCMat(), "Mavlink Not Connected",
				cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
	}

	pTextPos->m_y += pTextPos->m_w;

	//Vehicle position
	sprintf(strBuf, "Attitude: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
			current_messages.attitude.roll, current_messages.attitude.pitch, current_messages.attitude.yaw);
	PUTTEXT(pTextPos->m_x, pTextPos->m_y, strBuf);
	pTextPos->m_y += pTextPos->m_w;

	sprintf(strBuf, "Speed: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
			current_messages.attitude.rollspeed,
			current_messages.attitude.pitchspeed,
			current_messages.attitude.yawspeed);
	PUTTEXT(pTextPos->m_x, pTextPos->m_y, strBuf);
	pTextPos->m_y += pTextPos->m_w;

	return true;
}





}



