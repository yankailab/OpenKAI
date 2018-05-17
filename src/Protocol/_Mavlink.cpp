#include "_Mavlink.h"
#include "../Utility/util.h"

namespace kai
{

_Mavlink::_Mavlink()
{
	m_pIO = NULL;
	m_systemID = 1;
	m_myComponentID = MAV_COMP_ID_PATHPLANNER;
	m_type = MAV_TYPE_ONBOARD_CONTROLLER;
	m_targetComponentID = 0;
	m_msg.init();
}

_Mavlink::~_Mavlink()
{
	reset();
}

bool _Mavlink::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	//init param
	m_systemID = 1;
	m_myComponentID = MAV_COMP_ID_MISSIONPLANNER;//MAV_COMP_ID_PATHPLANNER;
	m_type = MAV_TYPE_GCS;//ONBOARD_CONTROLLER;
	m_targetComponentID = 0;
	m_msg.sysid = 0;
	m_msg.compid = 0;
	m_status.packet_rx_drop_count = 0;

	m_vPeer.clear();

	return true;
}

bool _Mavlink::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pIO,"_IOBase not found");

	Kiss** pItr = pK->getChildItr();
	int i=0;
	while (pItr[i])
	{
		Kiss* pP = pItr[i];
		IF_F(i >= MAV_N_PEER);
		i++;

		MAVLINK_PEER mP;
		mP.init();

		iName = "";
		F_ERROR_F(pP->v("_Mavlink", &iName));
		mP.m_pPeer = pK->root()->getChildInstByName(&iName);
		if(!mP.m_pPeer)
		{
			LOG_I("_Mavlink not found: " + iName);
			continue;
		}

		m_vPeer.push_back(mP);
	}
	return true;
}

void _Mavlink::reset()
{
	this->_ThreadBase::reset();
	m_pIO = NULL;
}

bool _Mavlink::start(void)
{
	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Mavlink::update(void)
{
	while (m_bThreadON)
	{
		if(!m_pIO)
		{
			this->sleepTime(USEC_1SEC);
			continue;
		}

		if(!m_pIO->isOpen())
		{
			this->sleepTime(USEC_1SEC);
			continue;
		}

		this->autoFPSfrom();

		handleMessages();

		this->autoFPSto();
	}
}

void _Mavlink::writeMessage(mavlink_message_t message)
{
	NULL_(m_pIO);

	IO_BUF ioB;
	ioB.m_nB = mavlink_msg_to_send_buffer(ioB.m_pB, &message);

	if(m_pIO->ioType()!=io_webSocket)
	{
		m_pIO->write(ioB);
	}
	else
	{
		_WebSocket* pWS = (_WebSocket*)m_pIO;
		pWS->write(ioB.m_pB, ioB.m_nB);
	}
}

void _Mavlink::sendHeartbeat(void)
{
	mavlink_message_t message;
	mavlink_msg_heartbeat_pack(m_systemID, m_myComponentID, &message, m_type, 0,
			0, 0, MAV_STATE_ACTIVE);

	writeMessage(message);

	LOG_I("<- HEARTBEAT");
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
	mavlink_msg_request_data_stream_encode(m_systemID, m_myComponentID, &message, &ds);

	writeMessage(message);

	LOG_I("<- REQUEST_DATA_STREAM");
}

void _Mavlink::gpsInput(mavlink_gps_input_t* pGPSinput)
{
	NULL_(pGPSinput);

	mavlink_message_t message;
	mavlink_msg_gps_input_encode(m_systemID, m_myComponentID, &message, pGPSinput);

	writeMessage(message);

	LOG_I("<- GPS_INPUT");
}

void _Mavlink::setAttitudeTarget(float* pAtti, float* pRate, float thrust,
		uint8_t mask)
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
	mavlink_msg_set_attitude_target_encode(m_systemID, m_myComponentID, &message, &ds);

	writeMessage(message);

	LOG_I("<- SET_ATTITUDE_TARGET: ROLL:" + f2str(pAtti[0]) +
			" PITCH:" + f2str(pAtti[1]) +
			" YAW:" + f2str(pAtti[2]) +
			" THR:" + f2str(thrust));
}

void _Mavlink::landingTarget(uint8_t stream_id, uint8_t frame, float angle_x,
		float angle_y, float distance, float size_x, float size_y)
{
	mavlink_message_t message;
	mavlink_landing_target_t ds;

	ds.time_usec = getTimeUsec();
	ds.target_num = 0;
	ds.frame = MAV_FRAME_BODY_NED;
	ds.angle_x = angle_x;
	ds.angle_y = angle_y;
	ds.distance = distance;
	ds.size_x = size_x;
	ds.size_y = size_y;
	mavlink_msg_landing_target_encode(m_systemID, m_myComponentID, &message, &ds);

	writeMessage(message);

	LOG_I("<- LANDING_TARGET: ANGLE_X:" + f2str(angle_x) + " ANGLE_Y:" + f2str(angle_y));
}

void _Mavlink::cmdLongDoSetMode(int mode)
{
	mavlink_message_t message;
	mavlink_command_long_t ds;

	ds.target_system = m_systemID;
	ds.target_component = m_targetComponentID;
	ds.command = MAV_CMD_DO_SET_MODE;
	ds.param1 = mode;
	mavlink_msg_command_long_encode(m_systemID, m_myComponentID, &message, &ds);

	writeMessage(message);
	LOG_I("<- cmdLongDoSetMode: "+i2str(mode));
}

void _Mavlink::cmdLongComponentArmDisarm(int p)
{
	mavlink_message_t message;
	mavlink_command_long_t ds;

	ds.target_system = m_systemID;
	ds.target_component = m_targetComponentID;
	ds.command = MAV_CMD_COMPONENT_ARM_DISARM;
	ds.param1 = p;
	mavlink_msg_command_long_encode(m_systemID, m_myComponentID, &message, &ds);

	writeMessage(message);
	LOG_I("<- cmdLongComponentArmDisarm: "+i2str(p));
}

void _Mavlink::commandLongDoSetPositionYawThrust(float steer, float thrust)
{
	mavlink_message_t message;
	mavlink_command_long_t ds;

	ds.target_system = m_systemID;
	ds.target_component = m_targetComponentID;
	ds.command = 213; //MAV_CMD_DO_SET_POSITION_YAW_THRUST;
	ds.confirmation = 0;
	ds.param1 = steer;
	ds.param2 = thrust;
	mavlink_msg_command_long_encode(m_systemID, m_myComponentID, &message,
			&ds);

	writeMessage(message);
	LOG_I("<- COMMAND_LONG: MAV_CMD_DO_SET_POSITION_YAW_THRUST");
}

void _Mavlink::distanceSensor(uint8_t type, uint8_t orientation, uint16_t max, uint16_t min, uint16_t v)
{
	/*
	 time_boot_ms: anything (it’s ignored)
	 min_distance: 100 (i.e. 1m)
	 max_distance: 1500 (i.e. 15m)
	 current_distance: depth-from-zed-in-cm
	 type: 0 (ignored)
	 id: 0 (also ignored for now)
	 orientation: 0 (means pointing forward)
	 covariance: 255 (ignored for now)
	 */

	mavlink_message_t message;
	mavlink_distance_sensor_t ds;
	ds.type = type;
	ds.max_distance = max;	//unit: centimeters
	ds.min_distance = min;
	ds.current_distance = v;
	ds.orientation = orientation;
	ds.covariance = 255;
	ds.id = 0;
	ds.time_boot_ms = getTimeBootMs();

	mavlink_msg_distance_sensor_encode(m_systemID, m_myComponentID, &message, &ds);

	writeMessage(message);
	LOG_I("<- DIST_SENSOR sysID = " + i2str(m_systemID) +
			", orient = " + i2str((int)orientation) +
			", d = " + i2str((int)ds.current_distance) +
			", min = " + i2str((int)ds.min_distance) +
			", max = " + i2str((int)ds.max_distance));
}

void _Mavlink::visionPositionDelta(uint64_t dTime, vDouble3* pDAngle,
		vDouble3* pDPos, uint8_t confidence)
{

	/*
	 * float angle_delta[3];
	 * Rotation in radians in body frame from previous to current frame
	 * using right-hand coordinate system (0=roll, 1=pitch, 2=yaw)
	 *
	 * float position_delta[3];
	 * Change in position in meters from previous to current frame
	 * rotated into body frame (0=forward, 1=right, 2=down)
	 *
	 * float confidence; //< normalised confidence value from 0 to 100
	 * */

	mavlink_message_t message;
	mavlink_vision_position_delta_t dZed;
	dZed.time_usec = getTimeUsec();
	dZed.time_delta_usec = dTime;
	dZed.angle_delta[0] = (float) pDAngle->x;
	dZed.angle_delta[1] = (float) pDAngle->y;
	dZed.angle_delta[2] = (float) pDAngle->z;
	dZed.position_delta[0] = (float) pDPos->x;
	dZed.position_delta[1] = (float) pDPos->y;
	dZed.position_delta[2] = (float) pDPos->z;
	dZed.confidence = (float) confidence;

	mavlink_msg_vision_position_delta_encode(m_systemID,
			m_myComponentID, &message, &dZed);

	writeMessage(message);
	LOG_I("<- VISION_POSITION_DELTA dT=" + i2str(dTime)
			+ ", forward=" + i2str(pDPos->x)
			+ ", right=" + i2str(pDPos->y)
			+ ", down=" + i2str(pDPos->z)
			+ "; roll=" + i2str(pDAngle->x)
			+ ", pitch=" + i2str(pDAngle->y)
			+ ", yaw=" + i2str(pDAngle->z)
			+ ", confidence=" + i2str(dZed.confidence));
}

void _Mavlink::positionTargetLocalNed(mavlink_position_target_local_ned_t* pD)
{
	/**
	 * @brief Pack a position_target_local_ned message
	 * @param system_id ID of this system
	 * @param component_id ID of this component (e.g. 200 for IMU)
	 * @param msg The MAVLink message to compress the data into
	 *
	 * @param time_boot_ms Timestamp in milliseconds since system boot
	 * @param coordinate_frame Valid options are: MAV_FRAME_LOCAL_NED = 1, MAV_FRAME_LOCAL_OFFSET_NED = 7, MAV_FRAME_BODY_NED = 8, MAV_FRAME_BODY_OFFSET_NED = 9
	 * @param type_mask Bitmask to indicate which dimensions should be ignored by the vehicle: a value of 0b0000000000000000 or 0b0000001000000000 indicates that none of the setpoint dimensions should be ignored. If bit 10 is set the floats afx afy afz should be interpreted as force instead of acceleration. Mapping: bit 1: x, bit 2: y, bit 3: z, bit 4: vx, bit 5: vy, bit 6: vz, bit 7: ax, bit 8: ay, bit 9: az, bit 10: is force setpoint, bit 11: yaw, bit 12: yaw rate
	 * @param x X Position in NED frame in meters
	 * @param y Y Position in NED frame in meters
	 * @param z Z Position in NED frame in meters (note, altitude is negative in NED)
	 * @param vx X velocity in NED frame in meter / s
	 * @param vy Y velocity in NED frame in meter / s
	 * @param vz Z velocity in NED frame in meter / s
	 * @param afx X acceleration or force (if bit 10 of type_mask is set) in NED frame in meter / s^2 or N
	 * @param afy Y acceleration or force (if bit 10 of type_mask is set) in NED frame in meter / s^2 or N
	 * @param afz Z acceleration or force (if bit 10 of type_mask is set) in NED frame in meter / s^2 or N
	 * @param yaw yaw setpoint in rad
	 * @param yaw_rate yaw rate setpoint in rad/s
	 * @return length of the message in bytes (excluding serial stream start sign)
	 */

	NULL_(pD);

	mavlink_message_t message;
	mavlink_msg_position_target_local_ned_encode(
			m_systemID,
			m_myComponentID, &message, pD);

	writeMessage(message);
	LOG_I("<- POS_TARGET_LOCAL_NED x=" + i2str(pD->x)
			+ ", y=" + i2str(pD->y)
			+ ", z=" + i2str(pD->z)
			+ ", vx=" + i2str(pD->vx)
			+ ", vy=" + i2str(pD->vy)
			+ ", vz=" + i2str(pD->vz)
			+ ", afx=" + i2str(pD->afx)
			+ ", afy=" + i2str(pD->afy)
			+ ", afz=" + i2str(pD->afz)
			+ ", yaw=" + i2str(pD->yaw)
			+ ", yawRate=" + i2str(pD->yaw_rate)
			+ ", cFrame=" + i2str(pD->coordinate_frame)
			+ ", typeMask=" + i2str(pD->type_mask)
			);
}

void _Mavlink::positionTargetGlobalInt(mavlink_position_target_global_int_t* pD)
{
	/**
	 * @brief Pack a position_target_global_int message
	 * @param system_id ID of this system
	 * @param component_id ID of this component (e.g. 200 for IMU)
	 * @param msg The MAVLink message to compress the data into
	 *
	 * @param time_boot_ms Timestamp in milliseconds since system boot. The rationale for the timestamp in the setpoint is to allow the system to compensate for the transport delay of the setpoint. This allows the system to compensate processing latency.
	 * @param coordinate_frame Valid options are: MAV_FRAME_GLOBAL_INT = 5, MAV_FRAME_GLOBAL_RELATIVE_ALT_INT = 6, MAV_FRAME_GLOBAL_TERRAIN_ALT_INT = 11
	 * @param type_mask Bitmask to indicate which dimensions should be ignored by the vehicle: a value of 0b0000000000000000 or 0b0000001000000000 indicates that none of the setpoint dimensions should be ignored. If bit 10 is set the floats afx afy afz should be interpreted as force instead of acceleration. Mapping: bit 1: x, bit 2: y, bit 3: z, bit 4: vx, bit 5: vy, bit 6: vz, bit 7: ax, bit 8: ay, bit 9: az, bit 10: is force setpoint, bit 11: yaw, bit 12: yaw rate
	 * @param lat_int X Position in WGS84 frame in 1e7 * meters
	 * @param lon_int Y Position in WGS84 frame in 1e7 * meters
	 * @param alt Altitude in meters in AMSL altitude, not WGS84 if absolute or relative, above terrain if GLOBAL_TERRAIN_ALT_INT
	 * @param vx X velocity in NED frame in meter / s
	 * @param vy Y velocity in NED frame in meter / s
	 * @param vz Z velocity in NED frame in meter / s
	 * @param afx X acceleration or force (if bit 10 of type_mask is set) in NED frame in meter / s^2 or N
	 * @param afy Y acceleration or force (if bit 10 of type_mask is set) in NED frame in meter / s^2 or N
	 * @param afz Z acceleration or force (if bit 10 of type_mask is set) in NED frame in meter / s^2 or N
	 * @param yaw yaw setpoint in rad
	 * @param yaw_rate yaw rate setpoint in rad/s
	 * @return length of the message in bytes (excluding serial stream start sign)
	 */

	NULL_(pD);

	mavlink_message_t message;
	mavlink_msg_position_target_global_int_encode(
			m_systemID,
			m_myComponentID, &message, pD);

	writeMessage(message);
	LOG_I("<- POS_TARGET_GLOBAL_INT lat=" + i2str(pD->lat_int)
			+ ", lng=" + i2str(pD->lon_int)
			+ ", alt=" + i2str(pD->alt)
			+ ", vx=" + i2str(pD->vx)
			+ ", vy=" + i2str(pD->vy)
			+ ", vz=" + i2str(pD->vz)
			+ ", afx=" + i2str(pD->afx)
			+ ", afy=" + i2str(pD->afy)
			+ ", afz=" + i2str(pD->afz)
			+ ", yaw=" + i2str(pD->yaw)
			+ ", yawRate=" + i2str(pD->yaw_rate)
			+ ", cFrame=" + i2str(pD->coordinate_frame)
			+ ", typeMask=" + i2str(pD->type_mask)
			);
}

void _Mavlink::rcChannelsOverride(mavlink_rc_channels_override_t* pD)
{
	/**
	 * @brief Send a rc_channels_override message
	 * @param chan MAVLink channel to send the message
	 *
	 * @param target_system System ID
	 * @param target_component Component ID
	 * @param chan1_raw RC channel 1 value, in microseconds. A value of UINT16_MAX means to ignore this field.
	 * @param chan2_raw RC channel 2 value, in microseconds. A value of UINT16_MAX means to ignore this field.
	 * @param chan3_raw RC channel 3 value, in microseconds. A value of UINT16_MAX means to ignore this field.
	 * @param chan4_raw RC channel 4 value, in microseconds. A value of UINT16_MAX means to ignore this field.
	 * @param chan5_raw RC channel 5 value, in microseconds. A value of UINT16_MAX means to ignore this field.
	 * @param chan6_raw RC channel 6 value, in microseconds. A value of UINT16_MAX means to ignore this field.
	 * @param chan7_raw RC channel 7 value, in microseconds. A value of UINT16_MAX means to ignore this field.
	 * @param chan8_raw RC channel 8 value, in microseconds. A value of UINT16_MAX means to ignore this field.
	 */

	NULL_(pD);

	mavlink_message_t message;
	mavlink_msg_rc_channels_override_encode(
			255,//m_systemID,
			m_myComponentID,
			&message, pD);

	writeMessage(message);
	LOG_I("<- RC OVERRIDE, chan1=" + i2str(pD->chan1_raw)
			+ ", chan2=" + i2str(pD->chan2_raw)
			+ ", chan3=" + i2str(pD->chan3_raw)
			+ ", chan4=" + i2str(pD->chan4_raw)
			+ ", chan5=" + i2str(pD->chan5_raw)
			+ ", chan6=" + i2str(pD->chan6_raw)
			+ ", chan7=" + i2str(pD->chan7_raw)
			+ ", chan8=" + i2str(pD->chan8_raw)
			);
}

void _Mavlink::setMode(mavlink_set_mode_t* pD)
{
	/**
	 * @brief Encode a set_mode struct
	 * @param system_id ID of this system
	 * @param component_id ID of this component (e.g. 200 for IMU)
	 * @param msg The MAVLink message to compress the data into
	 * @param set_mode C-struct to read the message contents from
	 */

	NULL_(pD);

	mavlink_message_t message;
	mavlink_msg_set_mode_encode(
			m_systemID,
			m_myComponentID,
			&message, pD);

	writeMessage(message);
	LOG_I("<- setMode, base_mode=" + i2str(pD->base_mode)
			+ ", custom_mode=" + i2str(pD->custom_mode));

}

bool _Mavlink::readMessage(mavlink_message_t &message)
{
	uint8_t	rBuf[MAV_N_BUF];
	static int nRead = 0;
	static int iRead = 0;

	if(nRead == 0)
	{
		nRead = m_pIO->read(rBuf, MAV_N_BUF);
		IF_F(nRead <= 0);
		iRead = 0;
	}

	while(iRead < nRead)
	{
		mavlink_status_t status;
		uint8_t result = mavlink_frame_char(MAVLINK_COMM_0, rBuf[iRead], &message, &status);
		iRead++;

		if (result == 1)
		{
			//Good message decoded
			m_status = status;
			return true;
		}
		else if (result == 2)
		{
			//Bad CRC
			LOG_I("-> DROPPED PACKETS:" + i2str(status.packet_rx_drop_count));
		}
	}

	nRead = 0;
	return false;
}

void _Mavlink::handleMessages()
{
	mavlink_message_t message;

	//Handle Message while new message is received
	while (readMessage(message))
	{
		// Note this doesn't handle multiple message sources.
		m_msg.sysid = message.sysid;
		m_msg.compid = message.compid;

		uint64_t tNow = getTimeUsec();

		// Handle Message ID
		switch (message.msgid)
		{

		case MAVLINK_MSG_ID_HEARTBEAT:
		{
			LOG_I("-> MAVLINK_MSG_ID_HEARTBEAT");
			mavlink_msg_heartbeat_decode(&message, &(m_msg.heartbeat));
			m_msg.time_stamps.heartbeat = tNow;

			if (m_msg.heartbeat.type == MAV_TYPE_GCS)break;

			m_systemID = m_msg.sysid;
			m_targetComponentID = m_msg.compid;

			LOG_I("SYSTEM_ID: " + i2str(m_systemID) +
  				  " COMPONENT_ID: " + i2str(m_myComponentID) +
				  " TARGET_COMPONENT_ID: " + i2str(m_targetComponentID));

			break;
		}

		case MAVLINK_MSG_ID_SYS_STATUS:
		{
			LOG_I("-> MAVLINK_MSG_ID_SYS_STATUS");
			mavlink_msg_sys_status_decode(&message, &(m_msg.sys_status));
			m_msg.time_stamps.sys_status = tNow;
			break;
		}

		case MAVLINK_MSG_ID_BATTERY_STATUS:
		{
			LOG_I("-> MAVLINK_MSG_ID_BATTERY_STATUS");
			mavlink_msg_battery_status_decode(&message,
					&(m_msg.battery_status));
			m_msg.time_stamps.battery_status = tNow;
			break;
		}

		case MAVLINK_MSG_ID_RADIO_STATUS:
		{
			LOG_I("-> MAVLINK_MSG_ID_RADIO_STATUS");
			mavlink_msg_radio_status_decode(&message, &(m_msg.radio_status));
			m_msg.time_stamps.radio_status = tNow;
			break;
		}

		case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
		{
			LOG_I("-> MAVLINK_MSG_ID_LOCAL_POSITION_NED");
			mavlink_msg_local_position_ned_decode(&message,
					&(m_msg.local_position_ned));
			m_msg.time_stamps.local_position_ned = tNow;
			break;
		}

		case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
		{
			LOG_I("-> MAVLINK_MSG_ID_GLOBAL_POSITION_INT");
			mavlink_msg_global_position_int_decode(&message,
					&(m_msg.global_position_int));
			m_msg.time_stamps.global_position_int = tNow;
			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
		{
			LOG_I("-> MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED");
			mavlink_msg_position_target_local_ned_decode(&message,
					&(m_msg.position_target_local_ned));
			m_msg.time_stamps.position_target_local_ned = tNow;
			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:
		{
			LOG_I("-> MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT");
			mavlink_msg_position_target_global_int_decode(&message,
					&(m_msg.position_target_global_int));
			m_msg.time_stamps.position_target_global_int = tNow;
			break;
		}

		case MAVLINK_MSG_ID_HIGHRES_IMU:
		{
			LOG_I("-> MAVLINK_MSG_ID_HIGHRES_IMU");
			mavlink_msg_highres_imu_decode(&message, &(m_msg.highres_imu));
			m_msg.time_stamps.highres_imu = tNow;
			break;
		}

		case MAVLINK_MSG_ID_ATTITUDE:
		{
			LOG_I("-> MAVLINK_MSG_ID_ATTITUDE");
			mavlink_msg_attitude_decode(&message, &(m_msg.attitude));
			m_msg.time_stamps.attitude = tNow;
			break;
		}

		case MAVLINK_MSG_ID_COMMAND_ACK:
		{
			mavlink_msg_command_ack_decode(&message, &(m_msg.command_ack));
			m_msg.time_stamps.attitude = tNow;

			LOG_I("-> MAVLINK_MSG_ID_COMMAND_ACK: " + i2str(m_msg.command_ack.result));
			break;
		}

		case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
		{
			mavlink_msg_rc_channels_override_decode(&message, &(m_msg.rc_channels_override));
			m_msg.time_stamps.rc_channels_override = tNow;

			LOG_I("-> RC OVERRIDE, systemID=" + i2str(m_msg.sysid)
					+ ", targetComponent=" + i2str(m_msg.compid)
					+ ", chan1=" + i2str(m_msg.rc_channels_override.chan1_raw)
					+ ", chan2=" + i2str(m_msg.rc_channels_override.chan2_raw)
					+ ", chan3=" + i2str(m_msg.rc_channels_override.chan3_raw)
					+ ", chan4=" + i2str(m_msg.rc_channels_override.chan4_raw)
					+ ", chan5=" + i2str(m_msg.rc_channels_override.chan5_raw)
					+ ", chan6=" + i2str(m_msg.rc_channels_override.chan6_raw)
					+ ", chan7=" + i2str(m_msg.rc_channels_override.chan7_raw)
					+ ", chan8=" + i2str(m_msg.rc_channels_override.chan8_raw)
					);
			break;
		}

		default:
		{
			LOG_I("-> UNKNOWN MSG_ID:" + i2str(message.msgid));
			break;
		}

		}

		//Message routing
		for(int i=0; i<m_vPeer.size(); i++)
		{
			MAVLINK_PEER* pMP = &m_vPeer[i];
			IF_CONT(!pMP->bCmdRoute(message.msgid));
			IF_CONT(message.msgid == MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE);

			_Mavlink* pM = (_Mavlink*)pMP->m_pPeer;
			IF_CONT(!pM);
			pM->writeMessage(message);
		}
	}
}

bool _Mavlink::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;

	string msg;

	if (!m_pIO->isOpen())
	{
		pWin->tabNext();
		msg = "Not Connected";
		pWin->addMsg(&msg);
		pWin->tabPrev();
		return true;
	}

	pWin->tabNext();

	msg = "y=" + f2str((double)m_msg.attitude.yaw) +
			" p=" + f2str((double)m_msg.attitude.pitch) +
			" r=" + f2str((double)m_msg.attitude.roll);
	pWin->addMsg(&msg);

	msg = "hdg=" + f2str(((double)m_msg.global_position_int.hdg)*0.01);
	pWin->addMsg(&msg);

	msg = "height=" + f2str(((double)m_msg.global_position_int.alt)*0.001);
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

bool _Mavlink::cli(int& iY)
{
	IF_F(!this->_ThreadBase::cli(iY));

	string msg;

	msg = "SYSTEM_ID: " + i2str(m_systemID);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "COMPONENT_ID: " + i2str(m_myComponentID);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "TARGET_COMPONENT_ID: " + i2str(m_targetComponentID);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}

