#include "_Mavlink.h"
#include "../Utility/util.h"

namespace kai
{

_Mavlink::_Mavlink()
{
	_ThreadBase();

	m_pSerialPort = NULL;
	m_systemID = 1;
	m_componentID = MAV_COMP_ID_PATHPLANNER;
	m_type = MAV_TYPE_ONBOARD_CONTROLLER;
	m_targetComponentID = 0;

	m_msg.attitude.pitch = 0;
	m_msg.attitude.roll = 0;
	m_msg.attitude.yaw = 0;
	m_msg.attitude.pitchspeed = 0;
	m_msg.attitude.rollspeed = 0;
	m_msg.attitude.yawspeed = 0;
}

_Mavlink::~_Mavlink()
{
	close();
}

bool _Mavlink::init(void* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	Kiss* pCC = pK->o("serialPort");
	CHECK_F(pCC->empty());
	m_pSerialPort = new SerialPort();
	CHECK_F(m_pSerialPort->init(pCC));

	//init param
	m_systemID = 1;
	m_componentID = MAV_COMP_ID_PATHPLANNER;
	m_type = MAV_TYPE_ONBOARD_CONTROLLER;
	m_targetComponentID = 0;
	m_msg.sysid = 0;
	m_msg.compid = 0;
	m_status.packet_rx_drop_count = 0;

	return true;
}

bool _Mavlink::link(void)
{
	NULL_F(m_pKiss);

	return true;
}

void _Mavlink::close()
{
	if (m_pSerialPort)
	{
		m_pSerialPort->close();
		delete m_pSerialPort;
		m_pSerialPort = NULL;
	}
}

void _Mavlink::handleMessages()
{
	mavlink_message_t message;
	int nMsgHandled = 0;

	//Handle Message while new message is received
	while (readMessage(message))
	{
		// Note this doesn't handle multiple message sources.
		m_msg.sysid = message.sysid;
		m_msg.compid = message.compid;

		// Handle Message ID
		switch (message.msgid)
		{

		case MAVLINK_MSG_ID_HEARTBEAT:
		{
#ifdef MAVLINK_DEBUG
			printf("-> MAVLINK_MSG_ID_HEARTBEAT\n");
#endif
			mavlink_msg_heartbeat_decode(&message, &(m_msg.heartbeat));
			m_msg.time_stamps.heartbeat = get_time_usec();

			if (m_msg.heartbeat.type != MAV_TYPE_GCS)
			{
				m_systemID = m_msg.sysid;
				m_targetComponentID = m_msg.compid;
#ifdef MAVLINK_DEBUG
				printf(	"-> SYSTEM_ID: %d;  COMPONENT_ID: %d;  TARGET_COMPONENT_ID: %d;\n",
						m_systemID, m_componentID, m_targetComponentID);
#endif
			}
			else
			{
#ifdef MAVLINK_DEBUG
				printf("-> HEARTBEAT FROM MAV_TYPE_GCS\n");
#endif
			}
			break;
		}

		case MAVLINK_MSG_ID_SYS_STATUS:
		{
#ifdef MAVLINK_DEBUG
			printf("-> MAVLINK_MSG_ID_SYS_STATUS\n");
#endif
			mavlink_msg_sys_status_decode(&message,
					&(m_msg.sys_status));
			m_msg.time_stamps.sys_status = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_BATTERY_STATUS:
		{
#ifdef MAVLINK_DEBUG
			printf("-> MAVLINK_MSG_ID_BATTERY_STATUS\n");
#endif
			mavlink_msg_battery_status_decode(&message,
					&(m_msg.battery_status));
			m_msg.time_stamps.battery_status = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_RADIO_STATUS:
		{
#ifdef MAVLINK_DEBUG
			printf("-> MAVLINK_MSG_ID_RADIO_STATUS\n");
#endif
			mavlink_msg_radio_status_decode(&message,
					&(m_msg.radio_status));
			m_msg.time_stamps.radio_status = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
		{
#ifdef MAVLINK_DEBUG
			printf("-> MAVLINK_MSG_ID_LOCAL_POSITION_NED\n");
#endif
			mavlink_msg_local_position_ned_decode(&message,
					&(m_msg.local_position_ned));
			m_msg.time_stamps.local_position_ned = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
		{
#ifdef MAVLINK_DEBUG
			printf("-> MAVLINK_MSG_ID_GLOBAL_POSITION_INT\n");
#endif
			mavlink_msg_global_position_int_decode(&message,
					&(m_msg.global_position_int));
			m_msg.time_stamps.global_position_int = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
		{
#ifdef MAVLINK_DEBUG
			printf("-> MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED\n");
#endif
			mavlink_msg_position_target_local_ned_decode(&message,
					&(m_msg.position_target_local_ned));
			m_msg.time_stamps.position_target_local_ned =
					get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:
		{
#ifdef MAVLINK_DEBUG
			printf("-> MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT\n");
#endif
			mavlink_msg_position_target_global_int_decode(&message,
					&(m_msg.position_target_global_int));
			m_msg.time_stamps.position_target_global_int =
					get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_HIGHRES_IMU:
		{
#ifdef MAVLINK_DEBUG
			printf("-> MAVLINK_MSG_ID_HIGHRES_IMU\n");
#endif
			mavlink_msg_highres_imu_decode(&message,
					&(m_msg.highres_imu));
			m_msg.time_stamps.highres_imu = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_ATTITUDE:
		{
#ifdef MAVLINK_DEBUG
			printf("-> MAVLINK_MSG_ID_ATTITUDE\n");
#endif
			mavlink_msg_attitude_decode(&message, &(m_msg.attitude));
			m_msg.time_stamps.attitude = get_time_usec();
			break;
		}

		case MAVLINK_MSG_ID_COMMAND_ACK:
		{
			mavlink_msg_command_ack_decode(&message,
					&(m_msg.command_ack));
			m_msg.time_stamps.attitude = get_time_usec();

#ifdef MAVLINK_DEBUG
			printf("-> MAVLINK_MSG_ID_COMMAND_ACK: %d \n",
					m_msg.command_ack.result);
#endif
			break;
		}

		default:
		{
#ifdef MAVLINK_DEBUG
			printf("-> MSG_ID: %i\n", message.msgid);
#endif
			break;
		}

		}

		if (++nMsgHandled >= NUM_MSG_HANDLE)
			return;
	}

}

bool _Mavlink::readMessage(mavlink_message_t &message)
{
	uint8_t cp;
	mavlink_status_t status;
	uint8_t result;

	while (m_pSerialPort->read(&cp, 1))
	{
//		if (mavlink_parse_char(MAVLINK_COMM_0, cp, &message, &status))
		result = mavlink_frame_char(MAVLINK_COMM_0, cp, &message, &status);

		if (result == 1)
		{
			//Good message decoded
			m_status = status;
			return true;
		}
		else if (result == 2)
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
	uint8_t buf[300];

	// Translate message to buffer
	unsigned int len = mavlink_msg_to_send_buffer(buf, &message);
	m_pSerialPort->write(buf, len);

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
		if(!m_pSerialPort->isOpen())
		{
			if(!m_pSerialPort->open())
			{
				this->sleepThread(USEC_1SEC);
				continue;
			}
			m_status.packet_rx_drop_count = 0;
		}

		this->autoFPSfrom();

		//Handling incoming messages
		handleMessages();

		this->autoFPSto();
	}

}

void _Mavlink::sendHeartbeat(void)
{
	mavlink_message_t message;
	mavlink_msg_heartbeat_pack(m_systemID, m_componentID, &message, m_type, 0,
			0, 0, MAV_STATE_ACTIVE);

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
	mavlink_msg_request_data_stream_encode(m_systemID, m_componentID, &message,
			&ds);

	writeMessage(message);

#ifdef MAVLINK_DEBUG
	printf("<- REQUEST_DATA_STREAM\n");
#endif

	return;
}

void _Mavlink::set_attitude_target(float* pAtti, float* pRate, float thrust, uint8_t mask)
{
	mavlink_message_t message;
	mavlink_set_attitude_target_t ds;

	//pAtti: Roll, Pitch, Yaw
	float pQ[4];
	mavlink_euler_to_quaternion(pAtti[0], pAtti[1], pAtti[2], pQ);

	ds.target_system = m_systemID;
	ds.target_component = m_targetComponentID;
	ds.q[0] = pQ[0];
	ds.q[1] = pQ[1];
	ds.q[2] = pQ[2];
	ds.q[3] = pQ[3];
	ds.body_roll_rate = pRate[0];
	ds.body_pitch_rate = pRate[1];
	ds.body_yaw_rate = pRate[2];
	ds.thrust = thrust;
	ds.type_mask = mask;

	mavlink_msg_set_attitude_target_encode(m_systemID, m_componentID, &message, &ds);

	writeMessage(message);

#ifdef MAVLINK_DEBUG
	printf("<- SET_ATTITUDE_TARGET: ROLL:%f, PITCH:%f, YAW:%f, THR:%f\n", pAtti[0], pAtti[1], pAtti[2], thrust);
#endif

	return;
}

void _Mavlink::landing_target(uint8_t stream_id, uint8_t frame, float angle_x,
		float angle_y, float distance, float size_x, float size_y)
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
	printf("<- LANDING_TARGET: ANGLE_X:%f; ANGLE_Y:%f;\n", angle_x, angle_y);
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
	printf("<- COMMAND_LONG: MAV_CMD_DO_SET_MODE\n");
#endif

	return;
}

void _Mavlink::command_long_doSetPositionYawThrust(float steer, float thrust)
{
	mavlink_message_t message;
	mavlink_command_long_t ds;

	ds.target_system = m_systemID;
	ds.target_component = m_targetComponentID;
	ds.command = 213; //MAV_CMD_DO_SET_POSITION_YAW_THRUST;
	ds.confirmation = 0;
	ds.param1 = steer;
	ds.param2 = thrust;
	mavlink_msg_command_long_encode(m_systemID, m_targetComponentID, &message,
			&ds);

	writeMessage(message);

#ifdef MAVLINK_DEBUG
	printf("<- COMMAND_LONG: MAV_CMD_DO_SET_POSITION_YAW_THRUST\n");
#endif

	return;
}

bool _Mavlink::draw(Frame* pFrame, vInt4* pTextPos)
{
	NULL_F(pFrame);
	Mat* pMat = pFrame->getCMat();

	if (m_pSerialPort->isOpen())
	{
		putText(*pMat, "Mavlink: Connected; FPS: " + i2str(getFrameRate()) + ", Mode: " + i2str(m_msg.heartbeat.custom_mode),
				cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX,
				0.5, Scalar(0, 255, 0), 1);
	}
	else
	{
		putText(*pMat, "Mavlink Not Connected",
				cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX,
				0.5, Scalar(0, 255, 0), 1);
	}

	pTextPos->m_y += pTextPos->m_w;

	//Vehicle position
//	char strBuf[512];
//	sprintf(strBuf, "Attitude: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
//			current_messages.attitude.roll, current_messages.attitude.pitch, current_messages.attitude.yaw);
//	PUTTEXT(pTextPos->m_x, pTextPos->m_y, strBuf);
//	pTextPos->m_y += pTextPos->m_w;
//
//	sprintf(strBuf, "Speed: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
//			current_messages.attitude.rollspeed,
//			current_messages.attitude.pitchspeed,
//			current_messages.attitude.yawspeed);
//	PUTTEXT(pTextPos->m_x, pTextPos->m_y, strBuf);
//	pTextPos->m_y += pTextPos->m_w;

	return true;
}

}

