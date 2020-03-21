#include "_Mavlink.h"
#include "../Utility/util.h"

namespace kai
{

_Mavlink::_Mavlink()
{
	m_pIO = NULL;
	m_mavMsg.init();

	m_mySystemID = 255;
	m_myComponentID = MAV_COMP_ID_MISSIONPLANNER;
	m_myType = MAV_TYPE_GCS;

	m_devSystemID = -1;
	m_devComponentID = -1;
	m_devType = 0;

	m_nRead = 0;
	m_iRead = 0;
}

_Mavlink::~_Mavlink()
{
}

bool _Mavlink::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("mySystemID", &m_mySystemID);
	pK->v("myComponentID", &m_myComponentID);
	pK->v("myType", &m_myType);

	pK->v("devSystemID", &m_devSystemID);
	pK->v("devComponentID", &m_devComponentID);
	pK->v("devType", &m_devType);

	m_status.packet_rx_drop_count = 0;

	string iName;

	iName = "";
	pK->v("_IOBase", &iName);
	m_pIO = (_IOBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pIO, "_IOBase not found");

	Kiss** pItr = pK->getChildItr();
	int i = 0;
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
		if (!mP.m_pPeer)
		{
			LOG_I("_Mavlink not found: " + iName);
			continue;
		}

		m_vPeer.push_back(mP);
	}

	//cmd routing
	vector<int> vNoRouteCmd;
	pK->a("noRouteCmd", &vNoRouteCmd);
	for (int i = 0; i < vNoRouteCmd.size(); i++)
	{
		setCmdRoute(vNoRouteCmd[i], false);
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
		if (!m_pIO)
		{
			this->sleepTime(USEC_1SEC);
			continue;
		}

		if (!m_pIO->isOpen())
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

	if (m_pIO->ioType() != io_webSocket)
	{
		m_pIO->write(pB, nB);
	}
	else
	{
		_WebSocket* pWS = (_WebSocket*) m_pIO;
		pWS->write(pB, nB, WS_MODE_BIN);
	}

	LOG_I(
			"<- MSG_ID = " + i2str((int )msg.msgid) + ", seq = "
					+ i2str((int )msg.seq));
}

void _Mavlink::cmdInt(mavlink_command_int_t& D)
{
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_command_int_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);

	LOG_I("<- CMD_INT command = " + i2str(D.command));
}

void _Mavlink::distanceSensor(mavlink_distance_sensor_t& D)
{
	/*
	 time_boot_ms: anything
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
	LOG_I(
			"<- DIST_SENSOR orient = " + i2str((int )D.orientation) + ", d = "
					+ i2str((int )D.current_distance) + ", min = "
					+ i2str((int )D.min_distance) + ", max = "
					+ i2str((int )D.max_distance));
}

void _Mavlink::globalVisionPositionEstimate(
		mavlink_global_vision_position_estimate_t& D)
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
	LOG_I(
			"<- GLOBAL_VISION_POSITION_ESTIMATE T=" + i2str(D.usec) + ", x="
					+ f2str(D.x) + ", y=" + f2str(D.y) + ", z=" + f2str(D.z)
					+ "; roll=" + f2str(D.roll) + ", pitch=" + f2str(D.pitch)
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
	LOG_I(
			"<- GPS_INPUT lat=" + i2str(D.lat) + ", lon=" + i2str(D.lon)
					+ ", alt=" + f2str(D.alt));
}

void _Mavlink::globalPositionInt(mavlink_global_position_int_t& D)
{
	D.time_boot_ms = getTimeBootMs();

	mavlink_message_t msg;
	mavlink_msg_global_position_int_encode(m_mySystemID, m_myComponentID, &msg,
			&D);

	writeMessage(msg);
	LOG_I(
			"<- GLOBAL_POS_INT lat=" + i2str(D.lat) + ", lon=" + i2str(D.lon)
					+ ", alt=" + i2str(D.alt) + ", relative_alt="
					+ i2str(D.relative_alt) + ", vx=" + i2str(D.vx) + ", vy="
					+ i2str(D.vy) + ", vz=" + i2str(D.vz) + ", hdg="
					+ i2str(D.hdg));
}

void _Mavlink::landingTarget(mavlink_landing_target_t& D)
{
	D.time_usec = getTimeUsec();

	mavlink_message_t msg;
	mavlink_msg_landing_target_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I(
			"<- landingTarget: angleX=" + f2str(D.angle_x) + ", angleY="
					+ f2str(D.angle_y) + ", distance=" + f2str(D.distance));
}

void _Mavlink::mountConfigure(mavlink_mount_configure_t& D)
{
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_mount_configure_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I(
			"<- mountConfigure: roll=" + i2str(D.stab_roll) + ", pitch="
					+ i2str(D.stab_pitch) + ", yaw=" + i2str(D.stab_yaw)
					+ ", mode=" + i2str(D.mount_mode));
}

void _Mavlink::mountControl(mavlink_mount_control_t& D)
{
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_mount_control_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I(
			"<- mountControl: pitch=" + i2str(D.input_a) + ", roll="
					+ i2str(D.input_b) + ", yaw=" + i2str(D.input_c)
					+ ", savePos=" + i2str(D.save_position));
}

void _Mavlink::mountStatus(mavlink_mount_status_t& D)
{
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_mount_status_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I(
			"<- mountControl: a=" + i2str(D.pointing_a) + ", b="
					+ i2str(D.pointing_b) + ", c=" + i2str(D.pointing_c));
}

void _Mavlink::param_set(mavlink_param_set_t& D)
{
	// mavlink_param_set_t D;
	// D.param_type = MAV_PARAM_TYPE_INT8;
	// D.param_value = 0;
	// string id = "WP_YAW_BEHAVIOR";
	// strcpy(D.param_id, id.c_str());
	// m_pAP->m_pMavlink->param_set(D);

	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_param_set_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);

	if (m_bLog)
	{
		char id[17];
		memcpy(id, D.param_id, 16);
		id[16] = 0;

		LOG_I(
				"<- paramSet: type=" + i2str(D.param_type) + ", value="
						+ f2str(D.param_value) + ", id=" + string(id));
	}
}

void _Mavlink::positionTargetLocalNed(mavlink_position_target_local_ned_t& D)
{
	D.time_boot_ms = getTimeBootMs();

	mavlink_message_t msg;
	mavlink_msg_position_target_local_ned_encode(m_mySystemID, m_myComponentID,
			&msg, &D);

	writeMessage(msg);
	LOG_I(
			"<- POS_TARGET_LOCAL_NED x=" + f2str(D.x) + ", y=" + f2str(D.y)
					+ ", z=" + f2str(D.z) + ", vx=" + f2str(D.vx) + ", vy="
					+ f2str(D.vy) + ", vz=" + f2str(D.vz) + ", afx="
					+ f2str(D.afx) + ", afy=" + f2str(D.afy) + ", afz="
					+ f2str(D.afz) + ", yaw=" + f2str(D.yaw) + ", yawRate="
					+ f2str(D.yaw_rate));
}

void _Mavlink::positionTargetGlobalInt(mavlink_position_target_global_int_t& D)
{
	mavlink_message_t msg;
	mavlink_msg_position_target_global_int_encode(m_mySystemID, m_myComponentID,
			&msg, &D);

	writeMessage(msg);
	LOG_I(
			"<- POS_TARGET_GLOBAL_INT lat=" + i2str(D.lat_int) + ", lng="
					+ i2str(D.lon_int) + ", alt=" + f2str(D.alt) + ", vx="
					+ f2str(D.vx) + ", vy=" + f2str(D.vy) + ", vz="
					+ f2str(D.vz) + ", afx=" + f2str(D.afx) + ", afy="
					+ f2str(D.afy) + ", afz=" + f2str(D.afz) + ", yaw="
					+ f2str(D.yaw) + ", yawRate=" + f2str(D.yaw_rate));
}

void _Mavlink::rcChannelsOverride(mavlink_rc_channels_override_t& D)
{
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_rc_channels_override_encode(m_mySystemID, m_myComponentID, &msg,
			&D);

	writeMessage(msg);
	LOG_I(
			"<- rcChannelsOverride, c1=" + i2str(D.chan1_raw) + ", c2="
					+ i2str(D.chan2_raw) + ", c3=" + i2str(D.chan3_raw)
					+ ", c4=" + i2str(D.chan4_raw) + ", c5="
					+ i2str(D.chan5_raw) + ", c6=" + i2str(D.chan6_raw)
					+ ", c7=" + i2str(D.chan7_raw) + ", c8="
					+ i2str(D.chan8_raw));
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
	mavlink_msg_request_data_stream_encode(m_mySystemID, m_myComponentID, &msg,
			&D);

	writeMessage(msg);
	LOG_I("<- requestDataStream");
}

void _Mavlink::heartbeat(void)
{
	mavlink_message_t msg;
	mavlink_msg_heartbeat_pack(m_mySystemID, m_myComponentID, &msg, m_myType, 0,
			0, 0, MAV_STATE_ACTIVE);

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
	mavlink_msg_set_attitude_target_encode(m_mySystemID, m_myComponentID, &msg,
			&D);

	writeMessage(msg);
	LOG_I("<- setTargetAttitude");
}

void _Mavlink::setMode(mavlink_set_mode_t& D)
{
	D.base_mode = 1;
	D.target_system = m_devSystemID;

	mavlink_message_t msg;
	mavlink_msg_set_mode_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I(
			"<- setMode, base_mode=" + i2str((int32_t )D.base_mode)
					+ ", custom_mode=" + i2str(D.custom_mode));
}

void _Mavlink::setPositionTargetLocalNED(
		mavlink_set_position_target_local_ned_t& D)
{
	D.time_boot_ms = getTimeBootMs();
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_set_position_target_local_ned_encode(m_mySystemID,
			m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I(
			"<- setPositionTargetLocalNED, x=" + f2str(D.x) + ", y="
					+ f2str(D.y) + ", z=" + f2str(D.z) + ", vx=" + f2str(D.vx)
					+ ", vy=" + f2str(D.vy) + ", vz=" + f2str(D.vz));
}

void _Mavlink::setPositionTargetGlobalINT(
		mavlink_set_position_target_global_int_t& D)
{
	D.time_boot_ms = getTimeBootMs();
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;

	mavlink_message_t msg;
	mavlink_msg_set_position_target_global_int_encode(m_mySystemID,
			m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I(
			"<- setPositionTargetGlobalINT, lat=" + i2str(D.lat_int) + ", lon="
					+ i2str(D.lon_int) + ", alt=" + f2str(D.alt) + ", vx="
					+ f2str(D.vx) + ", vy=" + f2str(D.vy) + ", vz="
					+ f2str(D.vz));
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

	mavlink_msg_vision_position_estimate_encode(m_mySystemID, m_myComponentID,
			&msg, &D);

	writeMessage(msg);
	LOG_I(
			"<- VISION_POSITION_ESTIMATE T=" + i2str(D.usec) + ", x="
					+ f2str(D.x) + ", y=" + f2str(D.y) + ", z=" + f2str(D.z)
					+ "; roll=" + f2str(D.roll) + ", pitch=" + f2str(D.pitch)
					+ ", yaw=" + f2str(D.yaw));
}

//CMD_LONG

void _Mavlink::clComponentArmDisarm(bool bArm)
{
	mavlink_command_long_t D;
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;
	D.command = MAV_CMD_COMPONENT_ARM_DISARM;
	D.param1 = (bArm) ? 1 : 0;

	mavlink_message_t msg;
	mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- cmdLongComponentArmDisarm: " + i2str(bArm));
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
	LOG_I("<- cmdLongDoSetMode: " + i2str(mode));
}

void _Mavlink::clNavSetYawSpeed(float yaw, float speed, float yawMode)
{
	mavlink_command_long_t D;
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;
	D.command = MAV_CMD_NAV_SET_YAW_SPEED;
	D.confirmation = 0;
	D.param1 = yaw;
	D.param2 = speed;
	D.param3 = yawMode;

	mavlink_message_t msg;
	mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- cmdLongDoSetPositionYawTrust: yaw=" + f2str(yaw) + ", speed=" + f2str(speed) + ", yawMode=" + f2str(yawMode));
}

void _Mavlink::clDoSetServo(int iServo, int PWM)
{
	mavlink_command_long_t D;
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;
	D.command = MAV_CMD_DO_SET_SERVO;
	D.param1 = iServo;
	D.param2 = (float) PWM;

	mavlink_message_t msg;
	mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I(
			"<- cmdLongDoSetServo: servo=" + i2str((int )iServo) + " pwm="
					+ i2str(PWM));
}

void _Mavlink::clDoSetRelay(int iRelay, bool bRelay)
{
	mavlink_command_long_t D;
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;
	D.command = MAV_CMD_DO_SET_RELAY;
	D.param1 = iRelay;
	D.param2 = (bRelay) ? 1.0 : 0.0;

	mavlink_message_t msg;
	mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I(
			"<- cmdLongDoSetRelay: relay=" + i2str((int )iRelay) + " relay="
					+ i2str((int )bRelay));
}

void _Mavlink::clGetHomePosition(void)
{
	mavlink_command_long_t D;
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;
	D.command = MAV_CMD_GET_HOME_POSITION;

	mavlink_message_t msg;
	mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- cmdLongGetHomePosition");
}

void _Mavlink::clNavTakeoff(float alt)
{
	mavlink_command_long_t D;
	D.target_system = m_devSystemID;
	D.target_component = m_devComponentID;
	D.command = MAV_CMD_NAV_TAKEOFF;
	D.param7 = alt;

	mavlink_message_t msg;
	mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

	writeMessage(msg);
	LOG_I("<- cmdNavTakeoff");
}

int16_t* _Mavlink::getRCinScaled(int iChan)
{
	switch (iChan)
	{
	case 1: return &m_mavMsg.m_rc_channels_scaled.chan1_scaled;
	case 2: return &m_mavMsg.m_rc_channels_scaled.chan2_scaled;
	case 3: return &m_mavMsg.m_rc_channels_scaled.chan3_scaled;
	case 4: return &m_mavMsg.m_rc_channels_scaled.chan4_scaled;
	case 5: return &m_mavMsg.m_rc_channels_scaled.chan5_scaled;
	case 6: return &m_mavMsg.m_rc_channels_scaled.chan6_scaled;
	case 7: return &m_mavMsg.m_rc_channels_scaled.chan7_scaled;
	case 8: return &m_mavMsg.m_rc_channels_scaled.chan8_scaled;
	default: return NULL;
	}
}

uint16_t* _Mavlink::getRCinRaw(int iChan)
{
	switch (iChan)
	{
	case 1: return &m_mavMsg.m_rc_channels.chan1_raw;
	case 2: return &m_mavMsg.m_rc_channels.chan2_raw;
	case 3: return &m_mavMsg.m_rc_channels.chan3_raw;
	case 4: return &m_mavMsg.m_rc_channels.chan4_raw;
	case 5: return &m_mavMsg.m_rc_channels.chan5_raw;
	case 6: return &m_mavMsg.m_rc_channels.chan6_raw;
	case 7: return &m_mavMsg.m_rc_channels.chan7_raw;
	case 8: return &m_mavMsg.m_rc_channels.chan8_raw;
	case 9: return &m_mavMsg.m_rc_channels.chan9_raw;
	case 10: return &m_mavMsg.m_rc_channels.chan10_raw;
	case 12: return &m_mavMsg.m_rc_channels.chan12_raw;
	case 13: return &m_mavMsg.m_rc_channels.chan13_raw;
	case 14: return &m_mavMsg.m_rc_channels.chan14_raw;
	case 15: return &m_mavMsg.m_rc_channels.chan15_raw;
	case 16: return &m_mavMsg.m_rc_channels.chan16_raw;
	case 17: return &m_mavMsg.m_rc_channels.chan17_raw;
	case 18: return &m_mavMsg.m_rc_channels.chan18_raw;
	default: return NULL;
	}
}

bool _Mavlink::readMessage(mavlink_message_t &msg)
{
	if (m_nRead == 0)
	{
		m_nRead = m_pIO->read(m_rBuf, N_IO_BUF);
		IF_F(m_nRead <= 0);
		m_iRead = 0;
	}

	while (m_iRead < m_nRead)
	{
		mavlink_status_t status;
		uint8_t result = mavlink_frame_char(MAVLINK_COMM_0, m_rBuf[m_iRead],
				&msg, &status);
		m_iRead++;

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

	m_nRead = 0;
	return false;
}

void _Mavlink::handleMessages()
{
	mavlink_message_t msg;

	while (readMessage(msg))
	{
		uint64_t tNow = getTimeUsec();

		if(m_devSystemID < 0)
			m_devSystemID = msg.sysid;

		if(m_devComponentID < 0)
			m_devComponentID = msg.compid;

		IF_CONT(msg.sysid != m_devSystemID);
		IF_CONT(msg.compid != m_devComponentID);

		switch (msg.msgid)
		{

		case MAVLINK_MSG_ID_ATTITUDE:
		{
			mavlink_msg_attitude_decode(&msg, &m_mavMsg.m_attitude);
			m_mavMsg.m_tStamps.m_attitude = tNow;
			LOG_I(" -> ATTITUDE");
			break;
		}

		case MAVLINK_MSG_ID_BATTERY_STATUS:
		{
			mavlink_msg_battery_status_decode(&msg, &m_mavMsg.m_battery_status);
			m_mavMsg.m_tStamps.m_battery_status = tNow;
			LOG_I(" -> BATTERY_STATUS");
			break;
		}

		case MAVLINK_MSG_ID_COMMAND_ACK:
		{
			mavlink_msg_command_ack_decode(&msg, &m_mavMsg.m_command_ack);
			m_mavMsg.m_tStamps.m_attitude = tNow;
			LOG_I(" -> COMMAND_ACK: " + i2str(m_mavMsg.m_command_ack.result));
			break;
		}

		case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
		{
			mavlink_msg_global_position_int_decode(&msg,
					&m_mavMsg.m_global_position_int);
			m_mavMsg.m_tStamps.m_global_position_int = tNow;
			LOG_I(" -> GLOBAL_POSITION_INT");
			break;
		}

		case MAVLINK_MSG_ID_HEARTBEAT:
		{
			mavlink_msg_heartbeat_decode(&msg, &m_mavMsg.m_heartbeat);
			m_mavMsg.m_tStamps.m_heartbeat = tNow;

			m_devType = m_mavMsg.m_heartbeat.type;

			LOG_I(
					" -> HEARTBEAT: sysid=" + i2str(msg.sysid) + ", compid="
							+ i2str((uint32_t )msg.compid) + ", seq="
							+ i2str((uint32_t )msg.seq));
			break;
		}

		case MAVLINK_MSG_ID_HIGHRES_IMU:
		{
			mavlink_msg_highres_imu_decode(&msg, &m_mavMsg.m_highres_imu);
			m_mavMsg.m_tStamps.m_highres_imu = tNow;
			LOG_I(" -> HIGHRES_IMU");
			break;
		}

		case MAVLINK_MSG_ID_HOME_POSITION:
		{
			mavlink_msg_home_position_decode(&msg, &m_mavMsg.m_home_position);
			m_mavMsg.m_tStamps.m_home_position = tNow;
			LOG_I(" -> HOME_POSITION");
			break;
		}

		case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
		{
			mavlink_msg_local_position_ned_decode(&msg,
					&m_mavMsg.m_local_position_ned);
			m_mavMsg.m_tStamps.m_local_position_ned = tNow;
			LOG_I(" -> LOCAL_POSITION_NED");
			break;
		}

		case MAVLINK_MSG_ID_MISSION_CURRENT:
		{
			mavlink_msg_mission_current_decode(&msg,
					&m_mavMsg.m_mission_current);
			m_mavMsg.m_tStamps.m_mission_current = tNow;
			LOG_I(" -> MISSION_CURRENT");
			break;
		}

		case MAVLINK_MSG_ID_MOUNT_STATUS:
		{
			mavlink_msg_mount_status_decode(&msg, &m_mavMsg.m_mount_status);
			m_mavMsg.m_tStamps.m_mount_status = tNow;
			LOG_I(" -> MOUNT_STATUS");
			break;
		}

		case MAVLINK_MSG_ID_PARAM_SET:
		{
			mavlink_msg_param_set_decode(&msg, &m_mavMsg.m_param_set);
			m_mavMsg.m_tStamps.m_param_set = tNow;

			if (m_bLog)
			{
				char id[17];
				memcpy(id, m_mavMsg.m_param_set.param_id, 16);
				id[16] = 0;

				LOG_I(
						" -> PARAM_SET: type="
								+ i2str(m_mavMsg.m_param_set.param_type)
								+ ", value="
								+ f2str(m_mavMsg.m_param_set.param_value)
								+ ", id=" + string(id));
			}

			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
		{
			mavlink_msg_position_target_local_ned_decode(&msg,
					&m_mavMsg.m_position_target_local_ned);
			m_mavMsg.m_tStamps.m_position_target_local_ned = tNow;
			LOG_I(" -> POSITION_TARGET_LOCAL_NED");
			break;
		}

		case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:
		{
			mavlink_msg_position_target_global_int_decode(&msg,
					&m_mavMsg.m_position_target_global_int);
			m_mavMsg.m_tStamps.m_position_target_global_int = tNow;
			LOG_I(" -> POSITION_TARGET_GLOBAL_INT");
			break;
		}

		case MAVLINK_MSG_ID_RC_CHANNELS:
		{
			mavlink_msg_rc_channels_decode(&msg, &(m_mavMsg.m_rc_channels));
			m_mavMsg.m_tStamps.m_rc_channels = tNow;

			LOG_I("-> RC_CHANNELS: chan1="
							+ i2str(m_mavMsg.m_rc_channels.chan1_raw)
							+ ", chan2="
							+ i2str(m_mavMsg.m_rc_channels.chan2_raw)
							+ ", chan3="
							+ i2str(m_mavMsg.m_rc_channels.chan3_raw)
							+ ", chan4="
							+ i2str(m_mavMsg.m_rc_channels.chan4_raw)
							+ ", chan5="
							+ i2str(m_mavMsg.m_rc_channels.chan5_raw)
							+ ", chan6="
							+ i2str(m_mavMsg.m_rc_channels.chan6_raw)
							+ ", chan7="
							+ i2str(m_mavMsg.m_rc_channels.chan7_raw)
							+ ", chan8="
							+ i2str(m_mavMsg.m_rc_channels.chan8_raw)
							+ ", chan9="
							+ i2str(m_mavMsg.m_rc_channels.chan9_raw)
							+ ", chan10="
							+ i2str(m_mavMsg.m_rc_channels.chan10_raw)
							+ ", chan11="
							+ i2str(m_mavMsg.m_rc_channels.chan11_raw)
							+ ", chan12="
							+ i2str(m_mavMsg.m_rc_channels.chan12_raw)
							+ ", chan13="
							+ i2str(m_mavMsg.m_rc_channels.chan13_raw)
							+ ", chan14="
							+ i2str(m_mavMsg.m_rc_channels.chan14_raw)
							+ ", chan15="
							+ i2str(m_mavMsg.m_rc_channels.chan15_raw)
							+ ", chan16="
							+ i2str(m_mavMsg.m_rc_channels.chan16_raw)
							+ ", chan17="
							+ i2str(m_mavMsg.m_rc_channels.chan17_raw)
							+ ", chan18="
							+ i2str(m_mavMsg.m_rc_channels.chan18_raw));
			break;
		}

		case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
		{
			mavlink_msg_rc_channels_override_decode(&msg,
					&(m_mavMsg.m_rc_channels_override));
			m_mavMsg.m_tStamps.m_rc_channels_override = tNow;

			LOG_I(
					" -> RC_OVERRIDE: chan1="
							+ i2str(m_mavMsg.m_rc_channels_override.chan1_raw)
							+ ", chan2="
							+ i2str(m_mavMsg.m_rc_channels_override.chan2_raw)
							+ ", chan3="
							+ i2str(m_mavMsg.m_rc_channels_override.chan3_raw)
							+ ", chan4="
							+ i2str(m_mavMsg.m_rc_channels_override.chan4_raw)
							+ ", chan5="
							+ i2str(m_mavMsg.m_rc_channels_override.chan5_raw)
							+ ", chan6="
							+ i2str(m_mavMsg.m_rc_channels_override.chan6_raw)
							+ ", chan7="
							+ i2str(m_mavMsg.m_rc_channels_override.chan7_raw)
							+ ", chan8="
							+ i2str(m_mavMsg.m_rc_channels_override.chan8_raw));
			break;
		}

		case MAVLINK_MSG_ID_RC_CHANNELS_SCALED:
		{
			mavlink_msg_rc_channels_scaled_decode(&msg,
					&(m_mavMsg.m_rc_channels_scaled));
			m_mavMsg.m_tStamps.m_rc_channels_scaled = tNow;

			LOG_I(
					" -> RC_SCALED: chan1="
							+ i2str(m_mavMsg.m_rc_channels_scaled.chan1_scaled)
							+ ", chan2="
							+ i2str(m_mavMsg.m_rc_channels_scaled.chan2_scaled)
							+ ", chan3="
							+ i2str(m_mavMsg.m_rc_channels_scaled.chan3_scaled)
							+ ", chan4="
							+ i2str(m_mavMsg.m_rc_channels_scaled.chan4_scaled)
							+ ", chan5="
							+ i2str(m_mavMsg.m_rc_channels_scaled.chan5_scaled)
							+ ", chan6="
							+ i2str(m_mavMsg.m_rc_channels_scaled.chan6_scaled)
							+ ", chan7="
							+ i2str(m_mavMsg.m_rc_channels_scaled.chan7_scaled)
							+ ", chan8="
							+ i2str(m_mavMsg.m_rc_channels_scaled.chan8_scaled));
			break;
		}

		case MAVLINK_MSG_ID_RC_CHANNELS_RAW:
		{
			mavlink_msg_rc_channels_raw_decode(&msg,
					&(m_mavMsg.m_rc_channels_raw));
			m_mavMsg.m_tStamps.m_rc_channels_raw = tNow;

			LOG_I(
					" -> RC_RAW: chan1="
							+ i2str(m_mavMsg.m_rc_channels_raw.chan1_raw)
							+ ", chan2="
							+ i2str(m_mavMsg.m_rc_channels_raw.chan2_raw)
							+ ", chan3="
							+ i2str(m_mavMsg.m_rc_channels_raw.chan3_raw)
							+ ", chan4="
							+ i2str(m_mavMsg.m_rc_channels_raw.chan4_raw)
							+ ", chan5="
							+ i2str(m_mavMsg.m_rc_channels_raw.chan5_raw)
							+ ", chan6="
							+ i2str(m_mavMsg.m_rc_channels_raw.chan6_raw)
							+ ", chan7="
							+ i2str(m_mavMsg.m_rc_channels_raw.chan7_raw)
							+ ", chan8="
							+ i2str(m_mavMsg.m_rc_channels_raw.chan8_raw));
			break;
		}

		case MAVLINK_MSG_ID_RADIO_STATUS:
		{
			mavlink_msg_radio_status_decode(&msg, &m_mavMsg.m_radio_status);
			m_mavMsg.m_tStamps.m_radio_status = tNow;
			LOG_I(" -> RADIO_STATUS");
			break;
		}

		case MAVLINK_MSG_ID_RAW_IMU:
		{
			mavlink_msg_raw_imu_decode(&msg, &m_mavMsg.m_raw_imu);
			m_mavMsg.m_tStamps.m_raw_imu = tNow;
			LOG_I(" -> RAW_IMU");
			break;
		}

		case MAVLINK_MSG_ID_SCALED_IMU:
		{
			mavlink_msg_scaled_imu_decode(&msg, &m_mavMsg.m_scaled_imu);
			m_mavMsg.m_tStamps.m_scaled_imu = tNow;
			LOG_I(" -> SCALED_IMU");
			break;
		}

		case MAVLINK_MSG_ID_SYS_STATUS:
		{
			mavlink_msg_sys_status_decode(&msg, &m_mavMsg.m_sys_status);
			m_mavMsg.m_tStamps.m_sys_status = tNow;
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
		for (int i = 0; i < m_vPeer.size(); i++)
		{
			MAVLINK_PEER* pMP = &m_vPeer[i];
			IF_CONT(!pMP->bCmdRoute(msg.msgid));

			_Mavlink* pM = (_Mavlink*) pMP->m_pPeer;
			IF_CONT(!pM);
			pM->writeMessage(msg);
		}
	}
}

void _Mavlink::setCmdRoute(uint32_t iCmd, bool bON)
{
	for (int i = 0; i < m_vPeer.size(); i++)
	{
		m_vPeer[i].setCmdRoute(iCmd, bON);
	}
}

void _Mavlink::draw(void)
{
	this->_ThreadBase::draw();

	string msg;

	if (!m_pIO->isOpen())
	{
		msg = "Not Connected";
		addMsg(msg, 1);
		return;
	}

	msg = "mySysID=" + i2str(m_mySystemID) + " myComID="
			+ i2str(m_myComponentID) + " myType=" + i2str(m_myType);
	addMsg(msg, 1);

	msg = "devSysID=" + i2str(m_devSystemID) + " devComID="
			+ i2str(m_devComponentID) + " devType=" + i2str(m_devType)
			+ " custom_mode=" + i2str((int) m_mavMsg.m_heartbeat.custom_mode);
	addMsg(msg, 1);
}

}
