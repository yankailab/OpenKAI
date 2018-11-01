#include "_Mavlink.h"
#include "../Utility/util.h"

namespace kai
{

_Mavlink::_Mavlink()
{
	m_pIO = NULL;
	m_msg.init();

	m_mySystemID = 255;
	m_myComponentID = MAV_COMP_ID_MISSIONPLANNER;
	m_myType = MAV_TYPE_GCS;

	m_devSystemID = -1;
	m_devComponentID = 0;
	m_devType = 0;
}

_Mavlink::~_Mavlink()
{
}

bool _Mavlink::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,mySystemID);
	KISSm(pK,myComponentID);
	KISSm(pK,myType);

	KISSm(pK,devSystemID);
	KISSm(pK,devComponentID);
	KISSm(pK,devType);

	m_msg.sysid = 0;
	m_msg.compid = 0;
	m_status.packet_rx_drop_count = 0;

	//link
	string iName;

	iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->root()->getChildInst(iName));
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
		mP.m_pPeer = pK->root()->getChildInst(iName);
		if(!mP.m_pPeer)
		{
			LOG_I("_Mavlink not found: " + iName);
			continue;
		}

		m_vPeer.push_back(mP);
	}

	//cmd routing
	int pNoRouteCmd[MAV_N_CMD];
	int m_nCmd = pK->array("noRouteCmd", pNoRouteCmd, MAV_N_CMD);
	IF_T(m_nCmd <= 0);
	for(int i=0; i<m_nCmd; i++)
	{
		setCmdRoute(pNoRouteCmd[i],false);
	}

	return true;
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

void _Mavlink::writeMessage(mavlink_message_t msg)
{
	NULL_(m_pIO);

	uint8_t pB[N_IO_BUF];
	int nB = mavlink_msg_to_send_buffer(pB, &msg);

	if(m_pIO->ioType()!=io_webSocket)
	{
		m_pIO->write(pB, nB);
	}
	else
	{
//		_WebSocket* pWS = (_WebSocket*)m_pIO;
//		pWS->write(ioB.m_pB, ioB.m_nB, WS_MODE_BIN);
	}
}

void _Mavlink::cmdInt(mavlink_command_int_t& D)
{
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_command_int_encode(
			m_mySystemID,
			m_myComponentID,
			&msg, &D);

	writeMessage(msg);

	LOG_I("<- CMD_INT command = " + i2str(D.command));
}

void _Mavlink::distanceSensor(mavlink_distance_sensor_t& D)
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

	D.id = 0;
	D.time_boot_ms = getTimeBootMs();

	mavlink_message_t msg;
	mavlink_msg_distance_sensor_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- DIST_SENSOR orient = " + i2str((int)D.orientation) +
			", d = " + i2str((int)D.current_distance) +
			", min = " + i2str((int)D.min_distance) +
			", max = " + i2str((int)D.max_distance));
}

void _Mavlink::globalVisionPositionEstimate(mavlink_global_vision_position_estimate_t& D)
{
	/*
	 *  uint64_t usec; // Timestamp (microseconds, synced to UNIX time or since system boot)
 float x; // Global X position
 float y; // Global Y position
 float z; // Global Z position
 float roll; // Roll angle in rad
 float pitch; // Pitch angle in rad
 float yaw; // Yaw angle in rad
	 */

	mavlink_message_t msg;
	D.usec = getTimeBootMs();

	mavlink_msg_global_vision_position_estimate_encode(m_mySystemID,
			m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- GLOBAL_VISION_POSITION_ESTIMATE T=" + i2str(D.usec)
			+ ", x=" + f2str(D.x)
			+ ", y=" + f2str(D.y)
			+ ", z=" + f2str(D.z)
			+ "; roll=" + f2str(D.roll)
			+ ", pitch=" + f2str(D.pitch)
			+ ", yaw=" + f2str(D.yaw));
}

void _Mavlink::gpsInput(mavlink_gps_input_t& D)
{
	/*
	time_week
	time_week_ms
	lat
	lon
	alt (optional)
	hdop (optinal)
	vdop (optinal)
	vn, ve, vd (optional)
	speed_accuracy (optional)
	horizontal_accuracy (optional)
	satellites_visible <-- required
	fix_type <-- required

typedef enum GPS_INPUT_IGNORE_FLAGS
{
	   GPS_INPUT_IGNORE_FLAG_ALT=1,
	   GPS_INPUT_IGNORE_FLAG_HDOP=2,
	   GPS_INPUT_IGNORE_FLAG_VDOP=4,
	   GPS_INPUT_IGNORE_FLAG_VEL_HORIZ=8,
	   GPS_INPUT_IGNORE_FLAG_VEL_VERT=16,
	   GPS_INPUT_IGNORE_FLAG_SPEED_ACCURACY=32,
	   GPS_INPUT_IGNORE_FLAG_HORIZONTAL_ACCURACY=64,
	   GPS_INPUT_IGNORE_FLAG_VERTICAL_ACCURACY=128,
	   GPS_INPUT_IGNORE_FLAGS_ENUM_END=129,
} GPS_INPUT_IGNORE_FLAGS;
	 */

	D.time_week = 1;
	D.time_week_ms = getTimeBootMs();

	mavlink_message_t msg;
	mavlink_msg_gps_input_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- GPS_INPUT lat=" + i2str(D.lat)
			+ ", lon=" + i2str(D.lon)
			+ ", alt=" + f2str(D.alt));
}

void _Mavlink::globalPositionInt(mavlink_global_position_int_t& D)
{
	D.time_boot_ms = getTimeBootMs();

	mavlink_message_t msg;
	mavlink_msg_global_position_int_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- GLOBAL_POS_INT lat=" + i2str(D.lat)
			+ ", lon=" + i2str(D.lon)
			+ ", alt=" + i2str(D.alt)
			+ ", relative_alt=" + i2str(D.relative_alt)
			+ ", vx=" + i2str(D.vx)
			+ ", vy=" + i2str(D.vy)
			+ ", vz=" + i2str(D.vz)
			+ ", hdg=" + i2str(D.hdg)
			);
}

void _Mavlink::landingTarget(mavlink_landing_target_t& D)
{
	D.time_usec = getTimeUsec();

	mavlink_message_t msg;
	mavlink_msg_landing_target_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- landingTarget: angleX=" + f2str(D.angle_x)
						 + ", angleY=" + f2str(D.angle_y)
						 + ", distance=" + f2str(D.distance));
}

void _Mavlink::mountConfigure(mavlink_mount_configure_t& D)
{
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_mount_configure_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- mountConfigure: roll=" + i2str(D.stab_roll)
						 + ", pitch=" + i2str(D.stab_pitch)
						 + ", yaw=" + i2str(D.stab_yaw)
						 + ", mode=" + i2str(D.mount_mode));
}

void _Mavlink::mountControl(mavlink_mount_control_t& D)
{
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_mount_control_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- mountControl: pitch=" + i2str(D.input_a)
						 + ", roll=" + i2str(D.input_b)
						 + ", yaw=" + i2str(D.input_c)
						 + ", savePos=" + i2str(D.save_position));
}

void _Mavlink::mountStatus(mavlink_mount_status_t& D)
{
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_mount_status_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- mountControl: a=" + i2str(D.pointing_a)
						 + ", b=" + i2str(D.pointing_b)
						 + ", c=" + i2str(D.pointing_c));
}

void _Mavlink::param_set(mavlink_param_set_t& D)
{
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_param_set_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);

	if(m_bLog)
	{
		char id[17];
		memcpy(id,D.param_id,16);
		id[16]=0;

		LOG_I("<- paramSet: type=" + i2str(D.param_type)
							 + ", value=" + f2str(D.param_value)
							 + ", id=" + string(id));
	}
}

void _Mavlink::positionTargetLocalNed(mavlink_position_target_local_ned_t& D)
{
	D.time_boot_ms = getTimeBootMs();

	mavlink_message_t msg;
	mavlink_msg_position_target_local_ned_encode(
			m_mySystemID,
			m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- POS_TARGET_LOCAL_NED x=" + i2str(D.x)
			+ ", y=" + i2str(D.y)
			+ ", z=" + i2str(D.z)
			+ ", vx=" + i2str(D.vx)
			+ ", vy=" + i2str(D.vy)
			+ ", vz=" + i2str(D.vz)
			+ ", afx=" + i2str(D.afx)
			+ ", afy=" + i2str(D.afy)
			+ ", afz=" + i2str(D.afz)
			+ ", yaw=" + i2str(D.yaw)
			+ ", yawRate=" + i2str(D.yaw_rate)
			+ ", cFrame=" + i2str(D.coordinate_frame)
			+ ", typeMask=" + i2str(D.type_mask)
			);
}

void _Mavlink::positionTargetGlobalInt(mavlink_position_target_global_int_t& D)
{
	mavlink_message_t msg;
	mavlink_msg_position_target_global_int_encode(
			m_mySystemID,
			m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- POS_TARGET_GLOBAL_INT lat=" + i2str(D.lat_int)
			+ ", lng=" + i2str(D.lon_int)
			+ ", alt=" + i2str(D.alt)
			+ ", vx=" + i2str(D.vx)
			+ ", vy=" + i2str(D.vy)
			+ ", vz=" + i2str(D.vz)
			+ ", afx=" + i2str(D.afx)
			+ ", afy=" + i2str(D.afy)
			+ ", afz=" + i2str(D.afz)
			+ ", yaw=" + i2str(D.yaw)
			+ ", yawRate=" + i2str(D.yaw_rate)
			+ ", cFrame=" + i2str(D.coordinate_frame)
			+ ", typeMask=" + i2str(D.type_mask)
			);
}

void _Mavlink::rcChannelsOverride(mavlink_rc_channels_override_t& D)
{
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_rc_channels_override_encode(
			m_mySystemID,
			m_myComponentID,
			&msg, &D);

	writeMessage(msg);
	LOG_I("<- rcChannelsOverride, c1=" + i2str(D.chan1_raw)
			+ ", c2=" + i2str(D.chan2_raw)
			+ ", c3=" + i2str(D.chan3_raw)
			+ ", c4=" + i2str(D.chan4_raw)
			+ ", c5=" + i2str(D.chan5_raw)
			+ ", c6=" + i2str(D.chan6_raw)
			+ ", c7=" + i2str(D.chan7_raw)
			+ ", c8=" + i2str(D.chan8_raw)
			);
}

void _Mavlink::requestDataStream(uint8_t stream_id, int rate)
{
	mavlink_request_data_stream_t D;
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;
	D.req_stream_id = stream_id;
	D.req_message_rate = rate;
	D.start_stop = 1;

	mavlink_message_t msg;
	mavlink_msg_request_data_stream_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- requestDataStream");
}

void _Mavlink::sendHeartbeat(void)
{
	mavlink_message_t msg;
	mavlink_msg_heartbeat_pack(
			m_mySystemID,
			m_myComponentID,
			&msg,
			m_myType,
			0, 0, 0, MAV_STATE_ACTIVE);

	writeMessage(msg);
	LOG_I("<- heartBeat");
}

void _Mavlink::setAttitudeTarget(mavlink_set_attitude_target_t& D)
{
	//  pAtti: Roll, Pitch, Yaw
	//	mavlink_euler_to_quaternion(pAtti[0], pAtti[1], pAtti[2], D.q);
	//	D.body_roll_rate = pRate[0];
	//	D.body_pitch_rate = pRate[1];
	//	D.body_yaw_rate = pRate[2];
	//	D.thrust = thrust;
	//	D.type_mask = mask;

	D.time_boot_ms = getTimeBootMs();
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_set_attitude_target_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- setTargetAttitude");
}

void _Mavlink::setMode(mavlink_set_mode_t& D)
{
	D.base_mode = 1;
	D.target_system = m_devSystemID;

	mavlink_message_t msg;
	mavlink_msg_set_mode_encode(
			m_mySystemID,
			m_myComponentID,
			&msg, &D);

	writeMessage(msg);
	LOG_I("<- setMode, base_mode=" + i2str(D.base_mode)
			+ ", custom_mode=" + i2str(D.custom_mode));
}

void _Mavlink::setPositionTargetLocalNED(mavlink_set_position_target_local_ned_t& D)
{
	D.time_boot_ms = getTimeBootMs();
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_set_position_target_local_ned_encode(
			m_mySystemID,
			m_myComponentID,
			&msg, &D);

	writeMessage(msg);
	LOG_I("<- setPositionTargetLocalNED, x=" + f2str(D.x)
			+ ", y=" + f2str(D.y)
			+ ", z=" + f2str(D.z)
			+ ", vx=" + f2str(D.vx)
			+ ", vy=" + f2str(D.vy)
			+ ", vz=" + f2str(D.vz)
			);
}

void _Mavlink::setPositionTargetGlobalINT(mavlink_set_position_target_global_int_t& D)
{
	D.time_boot_ms = getTimeBootMs();
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_set_position_target_global_int_encode(
			m_mySystemID,
			m_myComponentID,
			&msg, &D);

	writeMessage(msg);
	LOG_I("<- setPositionTargetGlobalINT, lat=" + i2str(D.lat_int)
			+ ", lon=" + i2str(D.lon_int)
			+ ", alt=" + f2str(D.alt)
			+ ", vx=" + f2str(D.vx)
			+ ", vy=" + f2str(D.vy)
			+ ", vz=" + f2str(D.vz)
			);
}

void _Mavlink::visionPositionDelta(mavlink_vision_position_delta_t& D)
{
	/*
	 * float angle_delta[3];
	 * Rotation in radians in body frame from previous to current frame
	 * using right-hand coordinate system (0=roll, 1=pitch, 2=yaw)
	 * float position_delta[3];
	 * Change in position in meters from previous to current frame
	 * rotated into body frame (0=forward, 1=right, 2=down)
	 * float confidence; //< normalised confidence value from 0 to 100
	D.time_delta_usec = dTime;
	D.angle_delta[0] = (float) pDAngle->x;
	D.angle_delta[1] = (float) pDAngle->y;
	D.angle_delta[2] = (float) pDAngle->z;
	D.position_delta[0] = (float) pDPos->x;
	D.position_delta[1] = (float) pDPos->y;
	D.position_delta[2] = (float) pDPos->z;
	D.confidence = (float) confidence;
	 * */

	mavlink_message_t msg;
	D.time_usec = getTimeUsec();

	mavlink_msg_vision_position_delta_encode(m_mySystemID,
			m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- VISION_POSITION_DELTA dT=" + i2str(D.time_delta_usec)
			+ ", forward=" + f2str(D.position_delta[0])
			+ ", right=" + f2str(D.position_delta[2])
			+ ", down=" + f2str(D.position_delta[0])
			+ "; roll=" + f2str(D.angle_delta[0])
			+ ", pitch=" + f2str(D.angle_delta[1])
			+ ", yaw=" + f2str(D.angle_delta[2])
			+ ", confidence=" + f2str(D.confidence));
}

void _Mavlink::visionPositionEstimate(mavlink_vision_position_estimate_t& D)
{
	/*
 uint64_t usec; // Timestamp (microseconds, synced to UNIX time or since system boot)
 float x; // Global X position
 float y; // Global Y position
 float z; // Global Z position
 float roll; // Roll angle in rad
 float pitch; // Pitch angle in rad
 float yaw; // Yaw angle in rad
	 */

	mavlink_message_t msg;
	D.usec = getTimeBootMs();

	mavlink_msg_vision_position_estimate_encode(m_mySystemID,
			m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- VISION_POSITION_ESTIMATE T=" + i2str(D.usec)
			+ ", x=" + f2str(D.x)
			+ ", y=" + f2str(D.y)
			+ ", z=" + f2str(D.z)
			+ "; roll=" + f2str(D.roll)
			+ ", pitch=" + f2str(D.pitch)
			+ ", yaw=" + f2str(D.yaw));
}

//CMD_LONG

void _Mavlink::clComponentArmDisarm(bool bArm)
{
	mavlink_command_long_t D;
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;
	D.command = MAV_CMD_COMPONENT_ARM_DISARM;
	D.param1 = (bArm)?1:0;

	mavlink_message_t msg;
	mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- cmdLongComponentArmDisarm: "+i2str(bArm));
}

void _Mavlink::clDoSetMode(int mode)
{
	mavlink_command_long_t D;
	D.target_system = m_mySystemID;
	D.target_component = m_devComponentID;
	D.command = MAV_CMD_DO_SET_MODE;
	D.param1 = mode;

	mavlink_message_t msg;
	mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- cmdLongDoSetMode: "+i2str(mode));
}

void _Mavlink::clDoSetPositionYawThrust(float steer, float thrust)
{
	mavlink_command_long_t D;
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;
	D.command = 213; //MAV_CMD_DO_SET_POSITION_YAW_THRUST;
	D.confirmation = 0;
	D.param1 = steer;
	D.param2 = thrust;

	mavlink_message_t msg;
	mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- cmdLongDoSetPositionYawTrust");
}

void _Mavlink::clDoSetServo(int iServo, int PWM)
{
	mavlink_command_long_t D;
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;
	D.command = MAV_CMD_DO_SET_SERVO;
	D.param1 = iServo;
	D.param2 = (float)PWM;

	mavlink_message_t msg;
	mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- cmdLongDoSetServo: servo="+i2str(iServo)
			+ " pwm=" + i2str(PWM));
}

bool _Mavlink::readMessage(mavlink_message_t &msg)
{
	static uint8_t rBuf[N_IO_BUF];
	static int nRead = 0;
	static int iRead = 0;

	if(nRead == 0)
	{
		nRead = m_pIO->read(rBuf, N_IO_BUF);
		IF_F(nRead <= 0);
		iRead = 0;
	}

	while(iRead < nRead)
	{
		mavlink_status_t status;
		uint8_t result = mavlink_frame_char(MAVLINK_COMM_0, rBuf[iRead], &msg, &status);
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
			LOG_I(" -> DROPPED PACKETS:" + i2str(status.packet_rx_drop_count));
		}
	}

	nRead = 0;
	return false;
}

void _Mavlink::handleMessages()
{
	mavlink_message_t msg;

	while (readMessage(msg))
	{
		uint64_t tNow = getTimeUsec();
		m_msg.sysid = msg.sysid;
		m_msg.compid = msg.compid;

		if(m_devSystemID > 0)
		{
			IF_CONT(m_msg.sysid != m_devSystemID);
		}

		switch (msg.msgid)
		{

		case MAVLINK_MSG_ID_ATTITUDE:
		{
			mavlink_msg_attitude_decode(&msg, &m_msg.attitude);
			m_msg.time_stamps.attitude = tNow;
			LOG_I(" -> ATTITUDE");
			break;
		}

		case MAVLINK_MSG_ID_BATTERY_STATUS:
		{
			mavlink_msg_battery_status_decode(&msg, &m_msg.battery_status);
			m_msg.time_stamps.battery_status = tNow;
			LOG_I(" -> BATTERY_STATUS");
			break;
		}

		case MAVLINK_MSG_ID_COMMAND_ACK:
		{
			mavlink_msg_command_ack_decode(&msg, &m_msg.command_ack);
			m_msg.time_stamps.attitude = tNow;
			LOG_I(" -> COMMAND_ACK: " + i2str(m_msg.command_ack.result));
			break;
		}

		case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
		{
			mavlink_msg_global_position_int_decode(&msg, &m_msg.global_position_int);
			m_msg.time_stamps.global_position_int = tNow;
			LOG_I(" -> GLOBAL_POSITION_INT");
			break;
		}

		case MAVLINK_MSG_ID_HEARTBEAT:
		{
			mavlink_msg_heartbeat_decode(&msg, &m_msg.heartbeat);
			m_msg.time_stamps.heartbeat = tNow;

			m_devSystemID = m_msg.sysid;
			m_devComponentID = m_msg.compid;
			m_devType = m_msg.heartbeat.type;

			LOG_I(" -> HEARTBEAT");
			break;
		}

		case MAVLINK_MSG_ID_HIGHRES_IMU:
		{
			mavlink_msg_highres_imu_decode(&msg, &m_msg.highres_imu);
			m_msg.time_stamps.highres_imu = tNow;
			LOG_I(" -> HIGHRES_IMU");
			break;
		}

		case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
		{
			mavlink_msg_local_position_ned_decode(&msg, &m_msg.local_position_ned);
			m_msg.time_stamps.local_position_ned = tNow;
			LOG_I(" -> LOCAL_POSITION_NED");
			break;
		}

		case MAVLINK_MSG_ID_MISSION_CURRENT:
		{
			mavlink_msg_mission_current_decode(&msg, &m_msg.mission_current);
			m_msg.time_stamps.mission_current = tNow;
			LOG_I(" -> MISSION_CURRENT");
			break;
		}

		case MAVLINK_MSG_ID_MOUNT_STATUS:
		{
			mavlink_msg_mount_status_decode(&msg, &m_msg.mount_status);
			m_msg.time_stamps.mount_status = tNow;
			LOG_I(" -> MOUNT_STATUS");
			break;
		}

		case MAVLINK_MSG_ID_PARAM_SET:
		{
			mavlink_msg_param_set_decode(&msg, &m_msg.param_set);
			m_msg.time_stamps.param_set = tNow;

			if(m_bLog)
			{
				static char id[17];
				memcpy(id,m_msg.param_set.param_id,16);
				id[16]=0;

				LOG_I(" -> PARAM_SET: type=" + i2str(m_msg.param_set.param_type)
									 + ", value=" + f2str(m_msg.param_set.param_value)
									 + ", id=" + string(id));
			}

			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
		{
			mavlink_msg_position_target_local_ned_decode(&msg, &m_msg.position_target_local_ned);
			m_msg.time_stamps.position_target_local_ned = tNow;
			LOG_I(" -> POSITION_TARGET_LOCAL_NED");
			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:
		{
			mavlink_msg_position_target_global_int_decode(&msg, &m_msg.position_target_global_int);
			m_msg.time_stamps.position_target_global_int = tNow;
			LOG_I(" -> POSITION_TARGET_GLOBAL_INT");
			break;
		}

		case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
		{
			mavlink_msg_rc_channels_override_decode(&msg, &(m_msg.rc_channels_override));
			m_msg.time_stamps.rc_channels_override = tNow;

			LOG_I(" -> RC_OVERRIDE: chan1=" + i2str(m_msg.rc_channels_override.chan1_raw)
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

		case MAVLINK_MSG_ID_RC_CHANNELS_RAW:
		{
			mavlink_msg_rc_channels_raw_decode(&msg, &(m_msg.rc_channels_raw));
			m_msg.time_stamps.rc_channels_raw = tNow;

			LOG_I(" -> RC_RAW: chan1=" + i2str(m_msg.rc_channels_raw.chan1_raw)
					+ ", chan2=" + i2str(m_msg.rc_channels_raw.chan2_raw)
					+ ", chan3=" + i2str(m_msg.rc_channels_raw.chan3_raw)
					+ ", chan4=" + i2str(m_msg.rc_channels_raw.chan4_raw)
					+ ", chan5=" + i2str(m_msg.rc_channels_raw.chan5_raw)
					+ ", chan6=" + i2str(m_msg.rc_channels_raw.chan6_raw)
					+ ", chan7=" + i2str(m_msg.rc_channels_raw.chan7_raw)
					+ ", chan8=" + i2str(m_msg.rc_channels_raw.chan8_raw)
					);
			break;
		}

		case MAVLINK_MSG_ID_RADIO_STATUS:
		{
			mavlink_msg_radio_status_decode(&msg, &m_msg.radio_status);
			m_msg.time_stamps.radio_status = tNow;
			LOG_I(" -> RADIO_STATUS");
			break;
		}

		case MAVLINK_MSG_ID_SYS_STATUS:
		{
			mavlink_msg_sys_status_decode(&msg, &m_msg.sys_status);
			m_msg.time_stamps.sys_status = tNow;
			LOG_I(" -> SYS_STATUS");
			break;
		}

		default:
		{
			LOG_I(" -> UNKNOWN MSG_ID:" + i2str(msg.msgid));
			break;
		}

		}

		//Message routing
		for(int i=0; i<m_vPeer.size(); i++)
		{
			MAVLINK_PEER* pMP = &m_vPeer[i];
			IF_CONT(!pMP->bCmdRoute(msg.msgid));

			_Mavlink* pM = (_Mavlink*)pMP->m_pPeer;
			IF_CONT(!pM);
			pM->writeMessage(msg);
		}
	}
}

void _Mavlink::setCmdRoute(uint32_t iCmd, bool bON)
{
	for(int i=0; i<m_vPeer.size(); i++)
	{
		m_vPeer[i].setCmdRoute(iCmd, bON);
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

	msg = "mySysID=" + i2str(m_mySystemID)
			+ " myComID=" + i2str(m_myComponentID)
			+ " myType=" + i2str(m_myType);
	pWin->addMsg(&msg);

	msg = "devSysID=" + i2str(m_devSystemID)
			+ " devComID=" + i2str(m_devComponentID)
			+ " devType=" + i2str(m_devType)
	 	 	+ " custom_mode=" + i2str((int)m_msg.heartbeat.custom_mode);
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

bool _Mavlink::cli(int& iY)
{
	IF_F(!this->_ThreadBase::cli(iY));

	string msg;

	if (!m_pIO->isOpen())
	{
		msg = "Not Connected";
		COL_MSG;
		iY++;
		mvaddstr(iY, CLI_X_MSG, msg.c_str());
		return true;
	}

	msg = "mySysID=" + i2str(m_mySystemID)
			+ " myComID=" + i2str(m_myComponentID)
			+ " myType=" + i2str(m_myType);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "devSysID=" + i2str(m_devSystemID)
			+ " devComID=" + i2str(m_devComponentID)
			+ " devType=" + i2str(m_devType)
	 	 	+ " custom_mode=" + i2str((int)m_msg.heartbeat.custom_mode);

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
