#include "_Mavlink.h"

namespace kai
{

	_Mavlink::_Mavlink()
	{
		m_pIO = nullptr;

		m_mySystemID = 255;
		m_myComponentID = MAV_COMP_ID_MISSIONPLANNER;
		m_myType = MAV_TYPE_GCS;

		m_devSystemID = -1;
		m_devComponentID = -1;
		m_devType = 0;

		m_nRead = 0;
		m_iRead = 0;
		m_iMavComm = MAVLINK_COMM_0;

		// msg register
		m_vpMsg.push_back(&m_attitude);
		m_vpMsg.push_back(&m_attitudeQuaternion);
		m_vpMsg.push_back(&m_batteryStatus);
		m_vpMsg.push_back(&m_commandAck);
		m_vpMsg.push_back(&m_globalPositionINT);
		m_vpMsg.push_back(&m_gpsRawINT);
		m_vpMsg.push_back(&m_heartbeat);
		m_vpMsg.push_back(&m_highresIMU);
		m_vpMsg.push_back(&m_homePosition);
		m_vpMsg.push_back(&m_localPositionNED);

		m_vpMsg.push_back(&m_missionAck);
		m_vpMsg.push_back(&m_missionClearAll);
		m_vpMsg.push_back(&m_missionCount);
		m_vpMsg.push_back(&m_missionCurrent);
		m_vpMsg.push_back(&m_missionItemInt);
		m_vpMsg.push_back(&m_missionItemReached);
		m_vpMsg.push_back(&m_missionRequestInt);
		m_vpMsg.push_back(&m_missionRequestList);
		m_vpMsg.push_back(&m_missionSetCurrent);

		m_vpMsg.push_back(&m_mountStatus);
		m_vpMsg.push_back(&m_paramRequestRead);
		m_vpMsg.push_back(&m_paramSet);
		m_vpMsg.push_back(&m_positionTargetLocalNED);
		m_vpMsg.push_back(&m_positionTargetGlobalINT);
		m_vpMsg.push_back(&m_radioStatus);
		m_vpMsg.push_back(&m_rawIMU);
		m_vpMsg.push_back(&m_rcChannels);
		m_vpMsg.push_back(&m_rcChannelsOverride);
		m_vpMsg.push_back(&m_servoOutputRaw);
		m_vpMsg.push_back(&m_statusText);
		m_vpMsg.push_back(&m_sysStatus);
		m_vpMsg.push_back(&m_scaledIMU);
	}

	_Mavlink::~_Mavlink()
	{
	}

	int _Mavlink::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("mySystemID", &m_mySystemID);
		pK->v("myComponentID", &m_myComponentID);
		pK->v("myType", &m_myType);

		pK->v("devSystemID", &m_devSystemID);
		pK->v("devComponentID", &m_devComponentID);
		pK->v("devType", &m_devType);

		pK->v("iMavComm", &m_iMavComm);

		m_status.packet_rx_drop_count = 0;

		return OK_OK;
	}

	int _Mavlink::link(void)
	{
		CHECK_(this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_IObase", &n);
		m_pIO = (_IObase *)(pK->findModule(n));
		NULL__(m_pIO, OK_ERR_NOT_FOUND);

		Kiss *pR = pK->child("routing");
		IF__(pR->empty(), OK_OK);

		int i = 0;
		while (1)
		{
			IF__(i >= MAV_N_PEER, OK_ERR_INVALID_VALUE);
			Kiss *pP = pR->child(i++);
			if (pP->empty())
				break;

			MAVLINK_PEER mP;
			mP.init();

			n = "";
			pP->v("_Mavlink", &n);
			mP.m_pPeer = pK->findModule(n);
			if (!mP.m_pPeer)
			{
				LOG_I("_Mavlink not found: " + n);
				continue;
			}

			m_vPeer.push_back(mP);
		}

		// cmd routing
		vector<int> vNoRouteCmd;
		pK->a("noRouteCmd", &vNoRouteCmd);
		for (int i = 0; i < vNoRouteCmd.size(); i++)
		{
			setCmdRoute(vNoRouteCmd[i], false);
		}

		return OK_OK;
	}

	bool _Mavlink::bConnected(void)
	{
		return (m_devSystemID >= 0);
	}

	void _Mavlink::setCmdRoute(uint32_t iCmd, bool bON)
	{
		for (unsigned int i = 0; i < m_vPeer.size(); i++)
		{
			m_vPeer[i].setCmdRoute(iCmd, bON);
		}
	}

	int _Mavlink::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _Mavlink::check(void)
	{
		NULL__(m_pIO, OK_ERR_NULLPTR);
		IF__(!m_pIO->bOpen(), OK_ERR_NOT_READY);

		return this->_ModuleBase::check();
	}

	void _Mavlink::update(void)
	{
		// Mavlink read and decode thread

		mavlink_message_t msg;

		while (m_pT->bAlive())
		{
			int r = readMessage(&msg);
			IF_CONT(r != OK_OK);

			if (m_devSystemID < 0)
				m_devSystemID = msg.sysid;

			if (m_devComponentID < 0)
				m_devComponentID = msg.compid;

			IF_CONT(msg.sysid != m_devSystemID);
			IF_CONT(msg.compid != m_devComponentID);

			// Decode
			bool bDecoded = false;
			for (MavMsgBase *pM : m_vpMsg)
			{
				IF_CONT(pM->m_id != msg.msgid);

				pM->decode(&msg);
				LOG_I(" -> Decoded MSG_ID:" + i2str(msg.msgid));
				bDecoded = true;
				break;
			}

			if (!bDecoded)
				LOG_I(" -> Unknown MSG_ID:" + i2str(msg.msgid));

			// Routing
			for (MAVLINK_PEER &p : m_vPeer)
			{
				//				IF_CONT(!p.bCmdRoute(msg.msgid));

				_Mavlink *pM = (_Mavlink *)p.m_pPeer;
				IF_CONT(!pM);
				pM->writeMessage(msg);
			}
		}
	}

	int _Mavlink::readMessage(mavlink_message_t *pMsg)
	{
		IF__(check() != OK_OK, OK_ERR_NOT_READY);
		NULL__(pMsg, OK_ERR_NULLPTR);

		if (m_nRead <= 0)
		{
			m_nRead = m_pIO->read(m_rBuf, MAV_N_BUF);
			IF__(m_nRead < 0, OK_ERR_UNKNOWN);
			IF__(m_nRead == 0, OK_ERR_NOT_READY);
			m_iRead = 0;
		}

		while (m_iRead < m_nRead)
		{
			uint8_t result = mavlink_frame_char(m_iMavComm,
												m_rBuf[m_iRead++],
												pMsg,
												&m_status);
			if (m_iRead == m_nRead)
			{
				m_iRead = 0;
				m_nRead = 0;
			}

			if (result == 1)
			{
				// Good message decoded
				return OK_OK;
			}
			else if (result == 2)
			{
				// Bad CRC
				LOG_I(" -> DROPPED PACKETS:" + i2str(m_status.packet_rx_drop_count));
			}
		}

		return OK_ERR_NOT_READY;
	}

	bool _Mavlink::writeMessage(const mavlink_message_t &msg)
	{
		NULL_F(m_pIO);

		uint8_t pB[MAV_N_BUF];
		int nB = mavlink_msg_to_send_buffer(pB, &msg);
		IF_F(!m_pIO->write(pB, nB));

		LOG_I("<- Wrote MSG_ID = " + i2str((int)msg.msgid) + ", seq = " + i2str((int)msg.seq));
		return true;
	}

	bool _Mavlink::cmdInt(mavlink_command_int_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_command_int_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I("<- CMD_INT command = " + i2str(D.command));
		return writeMessage(msg);
	}

	bool _Mavlink::distanceSensor(mavlink_distance_sensor_t &D)
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
		D.time_boot_ms = getTbootMs();

		mavlink_message_t msg;
		mavlink_msg_distance_sensor_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- DIST_SENSOR orient = " + i2str((int)D.orientation) + ", d = " + i2str((int)D.current_distance) + ", min = " + i2str((int)D.min_distance) + ", max = " + i2str((int)D.max_distance));
		return writeMessage(msg);
	}

	bool _Mavlink::globalPositionInt(mavlink_global_position_int_t &D)
	{
		D.time_boot_ms = getTbootMs();

		mavlink_message_t msg;
		mavlink_msg_global_position_int_encode(m_mySystemID, m_myComponentID, &msg,
											   &D);

		LOG_I(
			"<- GLOBAL_POS_INT lat=" + i2str(D.lat) + ", lon=" + i2str(D.lon) + ", alt=" + i2str(D.alt) + ", relative_alt=" + i2str(D.relative_alt) + ", vx=" + i2str(D.vx) + ", vy=" + i2str(D.vy) + ", vz=" + i2str(D.vz) + ", hdg=" + i2str(D.hdg));

		return writeMessage(msg);
	}

	bool _Mavlink::globalVisionPositionEstimate(
		mavlink_global_vision_position_estimate_t &D)
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
		D.usec = getTbootMs();

		mavlink_msg_global_vision_position_estimate_encode(m_mySystemID,
														   m_myComponentID, &msg, &D);

		LOG_I(
			"<- GLOBAL_VISION_POSITION_ESTIMATE T=" + i2str(D.usec) + ", x=" + f2str(D.x) + ", y=" + f2str(D.y) + ", z=" + f2str(D.z) + "; roll=" + f2str(D.roll) + ", pitch=" + f2str(D.pitch) + ", yaw=" + f2str(D.yaw));
		return writeMessage(msg);
	}

	bool _Mavlink::gpsInput(mavlink_gps_input_t &D)
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
		D.time_week_ms = getTbootMs();

		mavlink_message_t msg;
		mavlink_msg_gps_input_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- GPS_INPUT lat=" + i2str(D.lat) + ", lon=" + i2str(D.lon) + ", alt=" + f2str(D.alt));
		return writeMessage(msg);
	}

	bool _Mavlink::gpsRawINT(mavlink_gps_raw_int_t &D)
	{
		D.time_usec = getApproxTbootUs();

		mavlink_message_t msg;
		mavlink_msg_gps_raw_int_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I("<- GPS_RAW_INT lat=" + i2str(D.lat) + ", lon=" + i2str(D.lon) + ", alt=" + i2str(D.alt) + ", fix_type=" + i2str(D.fix_type));
		return writeMessage(msg);
	}

	bool _Mavlink::gpsRTCMdata(mavlink_gps_rtcm_data_t &D)
	{
		mavlink_message_t msg;
		mavlink_msg_gps_rtcm_data_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I("<- GPS_RTCM_DATA len=" + i2str(D.len));
		return writeMessage(msg);
	}

	bool _Mavlink::landingTarget(mavlink_landing_target_t &D)
	{
		D.time_usec = getApproxTbootUs();

		mavlink_message_t msg;
		mavlink_msg_landing_target_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- landingTarget: angleX=" + f2str(D.angle_x) + ", angleY=" + f2str(D.angle_y) + ", distance=" + f2str(D.distance));
		return writeMessage(msg);
	}

	bool _Mavlink::missionAck(mavlink_mission_ack_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_mission_ack_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- missionAck: type=" + i2str(D.mission_type) + ", opaqueID=" + i2str(D.opaque_id));
		return writeMessage(msg);
	}

	bool _Mavlink::missionClearAll(mavlink_mission_clear_all_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_mission_clear_all_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- missionClearAll: type=" + i2str(D.mission_type));
		return writeMessage(msg);
	}

	bool _Mavlink::missionCount(mavlink_mission_count_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_mission_count_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- missionCount: count=" + i2str(D.count) + ", type=" + i2str(D.mission_type) + ", opaqueID=" + i2str(D.opaque_id));
		return writeMessage(msg);
	}

	bool _Mavlink::missionCurrent(mavlink_mission_current_t &D)
	{
		mavlink_message_t msg;
		mavlink_msg_mission_current_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- missionCurrent: missionID=" + i2str(D.mission_id));
		return writeMessage(msg);
	}

	bool _Mavlink::missionItemInt(mavlink_mission_item_int_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_mission_item_int_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- missionItemInt");
		return writeMessage(msg);
	}

	bool _Mavlink::missionItemReached(mavlink_mission_item_reached_t &D)
	{
		mavlink_message_t msg;
		mavlink_msg_mission_item_reached_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- missionItemReached: seq=" + i2str(D.seq));
		return writeMessage(msg);
	}

	bool _Mavlink::missionRequestInt(mavlink_mission_request_int_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_mission_request_int_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- missionRequestInt: seq=" + i2str(D.seq) + ", type=" + i2str(D.mission_type));
		return writeMessage(msg);
	}

	bool _Mavlink::missionRequestList(mavlink_mission_request_list_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_mission_request_list_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- missionRequestList: type=" + i2str(D.mission_type));
		return writeMessage(msg);
	}

	bool _Mavlink::missionSetCurrent(mavlink_mission_set_current_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_mission_set_current_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- missionSetCurrent: seq=" + i2str(D.seq));
		return writeMessage(msg);
	}

	bool _Mavlink::mountConfigure(mavlink_mount_configure_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_mount_configure_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- mountConfigure: roll=" + i2str(D.stab_roll) + ", pitch=" + i2str(D.stab_pitch) + ", yaw=" + i2str(D.stab_yaw) + ", mode=" + i2str(D.mount_mode));
		return writeMessage(msg);
	}

	bool _Mavlink::mountControl(mavlink_mount_control_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_mount_control_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- mountControl: pitch=" + i2str(D.input_a) + ", roll=" + i2str(D.input_b) + ", yaw=" + i2str(D.input_c) + ", savePos=" + i2str(D.save_position));
		return writeMessage(msg);
	}

	bool _Mavlink::mountStatus(mavlink_mount_status_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_mount_status_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- mountStatus: a=" + i2str(D.pointing_a) + ", b=" + i2str(D.pointing_b) + ", c=" + i2str(D.pointing_c));
		return writeMessage(msg);
	}

	bool _Mavlink::paramRequestRead(mavlink_param_request_read_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_param_request_read_encode(m_mySystemID, m_myComponentID, &msg, &D);

		if (m_bLog)
		{
			char id[17];
			memcpy(id, D.param_id, 16);
			id[16] = 0;

			LOG_I(
				"<- paramRequestRead: id=" + string(D.param_id) + ", index=" + i2str((int)D.param_index));
		}

		return writeMessage(msg);
	}

	bool _Mavlink::paramSet(mavlink_param_set_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_param_set_encode(m_mySystemID, m_myComponentID, &msg, &D);

		if (m_bLog)
		{
			char id[17];
			memcpy(id, D.param_id, 16);
			id[16] = 0;

			LOG_I(
				"<- paramSet: type=" + i2str(D.param_type) + ", value=" + f2str(D.param_value) + ", id=" + string(id));
		}
		return writeMessage(msg);
	}

	bool _Mavlink::paramValue(mavlink_param_value_t &D)
	{
		mavlink_message_t msg;
		mavlink_msg_param_value_encode(m_mySystemID, m_myComponentID, &msg, &D);

		if (m_bLog)
		{
			char id[17];
			memcpy(id, D.param_id, 16);
			id[16] = 0;

			LOG_I(
				"<- paramValue: type=" + i2str(D.param_type) + ", value=" + f2str(D.param_value) + ", id=" + string(id));
		}
		return writeMessage(msg);
	}

	bool _Mavlink::positionTargetLocalNed(mavlink_position_target_local_ned_t &D)
	{
		D.time_boot_ms = getTbootMs();

		mavlink_message_t msg;
		mavlink_msg_position_target_local_ned_encode(m_mySystemID, m_myComponentID,
													 &msg, &D);

		LOG_I(
			"<- POS_TARGET_LOCAL_NED x=" + f2str(D.x) + ", y=" + f2str(D.y) + ", z=" + f2str(D.z) + ", vx=" + f2str(D.vx) + ", vy=" + f2str(D.vy) + ", vz=" + f2str(D.vz) + ", afx=" + f2str(D.afx) + ", afy=" + f2str(D.afy) + ", afz=" + f2str(D.afz) + ", yaw=" + f2str(D.yaw) + ", yawRate=" + f2str(D.yaw_rate));
		return writeMessage(msg);
	}

	bool _Mavlink::positionTargetGlobalInt(mavlink_position_target_global_int_t &D)
	{
		mavlink_message_t msg;
		mavlink_msg_position_target_global_int_encode(m_mySystemID, m_myComponentID,
													  &msg, &D);

		LOG_I(
			"<- POS_TARGET_GLOBAL_INT lat=" + i2str(D.lat_int) + ", lng=" + i2str(D.lon_int) + ", alt=" + f2str(D.alt) + ", vx=" + f2str(D.vx) + ", vy=" + f2str(D.vy) + ", vz=" + f2str(D.vz) + ", afx=" + f2str(D.afx) + ", afy=" + f2str(D.afy) + ", afz=" + f2str(D.afz) + ", yaw=" + f2str(D.yaw) + ", yawRate=" + f2str(D.yaw_rate));
		return writeMessage(msg);
	}

	bool _Mavlink::rcChannelsOverride(mavlink_rc_channels_override_t &D)
	{
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_rc_channels_override_encode(m_mySystemID, m_myComponentID, &msg,
												&D);

		LOG_I(
			"<- rcChannelsOverride, c1=" + i2str(D.chan1_raw) + ", c2=" + i2str(D.chan2_raw) + ", c3=" + i2str(D.chan3_raw) + ", c4=" + i2str(D.chan4_raw) + ", c5=" + i2str(D.chan5_raw) + ", c6=" + i2str(D.chan6_raw) + ", c7=" + i2str(D.chan7_raw) + ", c8=" + i2str(D.chan8_raw));
		return writeMessage(msg);
	}

	bool _Mavlink::requestDataStream(uint8_t stream_id, int rate)
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

		LOG_I("<- requestDataStream");
		return writeMessage(msg);
	}

	bool _Mavlink::heartbeat(void)
	{
		mavlink_message_t msg;
		mavlink_msg_heartbeat_pack(m_mySystemID, m_myComponentID, &msg, m_myType, 0,
								   0, 0, MAV_STATE_ACTIVE);

		LOG_I("<- heartBeat");
		return writeMessage(msg);
	}

	bool _Mavlink::setAttitudeTarget(mavlink_set_attitude_target_t &D)
	{
		//  pAtti: Roll, Pitch, Yaw
		//	mavlink_euler_to_quaternion(pAtti[0], pAtti[1], pAtti[2], D.q);
		//	D.body_roll_rate = pRate[0];
		//	D.body_pitch_rate = pRate[1];
		//	D.body_yaw_rate = pRate[2];
		//	D.thrust = thrust;
		//	D.type_mask = mask;

		D.time_boot_ms = getTbootMs();
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_set_attitude_target_encode(m_mySystemID, m_myComponentID, &msg,
											   &D);

		LOG_I("<- setTargetAttitude");
		return writeMessage(msg);
	}

	bool _Mavlink::setMode(mavlink_set_mode_t &D)
	{
		D.base_mode = 1;
		D.target_system = m_devSystemID;

		mavlink_message_t msg;
		mavlink_msg_set_mode_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- setMode, base_mode=" + i2str((int32_t)D.base_mode) + ", custom_mode=" + i2str(D.custom_mode));
		return writeMessage(msg);
	}

	bool _Mavlink::setPositionTargetLocalNED(
		mavlink_set_position_target_local_ned_t &D)
	{
		D.time_boot_ms = getTbootMs();
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_set_position_target_local_ned_encode(m_mySystemID,
														 m_myComponentID, &msg, &D);

		LOG_I(
			"<- setPositionTargetLocalNED, x=" + f2str(D.x) + ", y=" + f2str(D.y) + ", z=" + f2str(D.z) + ", vx=" + f2str(D.vx) + ", vy=" + f2str(D.vy) + ", vz=" + f2str(D.vz));
		return writeMessage(msg);
	}

	bool _Mavlink::setPositionTargetGlobalINT(
		mavlink_set_position_target_global_int_t &D)
	{
		D.time_boot_ms = getTbootMs();
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;

		mavlink_message_t msg;
		mavlink_msg_set_position_target_global_int_encode(m_mySystemID,
														  m_myComponentID, &msg, &D);

		LOG_I(
			"<- setPositionTargetGlobalINT, lat=" + i2str(D.lat_int) + ", lon=" + i2str(D.lon_int) + ", alt=" + f2str(D.alt) + ", vx=" + f2str(D.vx) + ", vy=" + f2str(D.vy) + ", vz=" + f2str(D.vz));
		return writeMessage(msg);
	}

	bool _Mavlink::statusText(mavlink_statustext_t &D)
	{
		mavlink_message_t msg;
		mavlink_msg_statustext_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- statusText: " + string(D.text));
		return writeMessage(msg);
	}

	bool _Mavlink::visionPositionEstimate(mavlink_vision_position_estimate_t &D)
	{
		mavlink_message_t msg;
		D.usec = getTbootMs();

		mavlink_msg_vision_position_estimate_encode(m_mySystemID, m_myComponentID,
													&msg, &D);

		LOG_I(
			"<- VISION_POSITION_ESTIMATE T=" + i2str(D.usec) + ", x=" + f2str(D.x) + ", y=" + f2str(D.y) + ", z=" + f2str(D.z) + "; roll=" + f2str(D.roll) + ", pitch=" + f2str(D.pitch) + ", yaw=" + f2str(D.yaw));
		return writeMessage(msg);
	}

	bool _Mavlink::visionSpeedEstimate(mavlink_vision_speed_estimate_t &D)
	{
		mavlink_message_t msg;
		D.usec = getTbootMs();

		mavlink_msg_vision_speed_estimate_encode(m_mySystemID, m_myComponentID,
												 &msg, &D);

		LOG_I(
			"<- VISION_SPEED_ESTIMATE T=" + i2str(D.usec) + ", x=" + f2str(D.x) + ", y=" + f2str(D.y) + ", z=" + f2str(D.z));
		return writeMessage(msg);
	}

	// CMD_LONG

	bool _Mavlink::clComponentArmDisarm(bool bArm)
	{
		mavlink_command_long_t D;
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;
		D.command = MAV_CMD_COMPONENT_ARM_DISARM;
		D.confirmation = 0;
		D.param1 = (bArm) ? 1 : 0;
		D.param2 = 0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		mavlink_message_t msg;
		mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I("<- cmdLongComponentArmDisarm: " + i2str(bArm));
		return writeMessage(msg);
	}

	bool _Mavlink::clDoFlightTermination(bool bTerminate)
	{
		mavlink_command_long_t D;
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;
		D.command = MAV_CMD_DO_FLIGHTTERMINATION;
		D.confirmation = 0;
		D.param1 = (bTerminate) ? 1 : 0;
		D.param2 = 0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		mavlink_message_t msg;
		mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I("<- cmdLongFlightTermination: " + i2str(bTerminate));
		return writeMessage(msg);
	}

	bool _Mavlink::clDoSetMode(int mode)
	{
		mavlink_command_long_t D;
		D.target_system = m_mySystemID;
		D.target_component = m_devComponentID;
		D.command = MAV_CMD_DO_SET_MODE;
		D.confirmation = 0;
		D.param1 = mode;
		D.param2 = 0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		mavlink_message_t msg;
		mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I("<- cmdLongDoSetMode: " + i2str(mode));
		return writeMessage(msg);
	}

	bool _Mavlink::clNavSetYawSpeed(float yaw, float speed, float yawMode)
	{
		mavlink_command_long_t D;
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;
		D.command = MAV_CMD_NAV_SET_YAW_SPEED;
		D.confirmation = 0;
		D.param1 = yaw;
		D.param2 = speed;
		D.param3 = yawMode;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		mavlink_message_t msg;
		mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I("<- cmdLongDoSetPositionYawTrust: yaw=" + f2str(yaw) + ", speed=" + f2str(speed) + ", yawMode=" + f2str(yawMode));
		return writeMessage(msg);
	}

	bool _Mavlink::clDoSetServo(int iServo, int PWM)
	{
		mavlink_command_long_t D;
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;
		D.command = MAV_CMD_DO_SET_SERVO;
		D.confirmation = 0;
		D.param1 = iServo;
		D.param2 = (float)PWM;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		mavlink_message_t msg;
		mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- cmdLongDoSetServo: servo=" + i2str((int)iServo) + " pwm=" + i2str(PWM));
		return writeMessage(msg);
	}

	bool _Mavlink::clDoSetRelay(int iRelay, bool bRelay)
	{
		mavlink_command_long_t D;
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;
		D.command = MAV_CMD_DO_SET_RELAY;
		D.confirmation = 0;
		D.param1 = iRelay;
		D.param2 = (bRelay) ? 1.0 : 0.0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		mavlink_message_t msg;
		mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I(
			"<- cmdLongDoSetRelay: relay=" + i2str((int)iRelay) + " relay=" + i2str((int)bRelay));
		return writeMessage(msg);
	}

	bool _Mavlink::clDoSetHome(bool bUseCurrent, float r, float p, float y, float lat, float lon, float alt)
	{
		mavlink_command_long_t D;
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;
		D.command = MAV_CMD_DO_SET_HOME;
		D.confirmation = 0;
		D.param1 = (bUseCurrent) ? 1 : 0;
		D.param2 = r;
		D.param3 = p;
		D.param4 = y;
		D.param5 = lat;
		D.param6 = lon;
		D.param7 = alt;

		mavlink_message_t msg;
		mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I("<- cmdLongSetHome");
		return writeMessage(msg);
	}

	bool _Mavlink::clGetHomePosition(void)
	{
		mavlink_command_long_t D;
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;
		D.command = MAV_CMD_GET_HOME_POSITION;
		D.confirmation = 0;
		D.param1 = 0;
		D.param2 = 0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		mavlink_message_t msg;
		mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I("<- cmdLongGetHomePosition");
		return writeMessage(msg);
	}

	bool _Mavlink::clNavTakeoff(float alt)
	{
		mavlink_command_long_t D;
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;
		D.command = MAV_CMD_NAV_TAKEOFF;
		D.confirmation = 0;
		D.param1 = 0;
		D.param2 = 0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = alt;

		mavlink_message_t msg;
		mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I("<- cmdNavTakeoff");
		return writeMessage(msg);
	}

	bool _Mavlink::clNavRTL(void)
	{
		mavlink_command_long_t D;
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;
		D.command = MAV_CMD_NAV_RETURN_TO_LAUNCH;
		D.confirmation = 0;
		D.param1 = 0;
		D.param2 = 0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		mavlink_message_t msg;
		mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I("<- cmdNavRTL");
		return writeMessage(msg);
	}

	bool _Mavlink::clSetMessageInterval(float id, float interval, float responseTarget)
	{
		mavlink_command_long_t D;
		D.target_system = m_devSystemID;
		D.target_component = m_devComponentID;
		D.command = MAV_CMD_SET_MESSAGE_INTERVAL;
		D.confirmation = 0;
		D.param1 = id;
		D.param2 = interval;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = responseTarget;

		mavlink_message_t msg;
		mavlink_msg_command_long_encode(m_mySystemID, m_myComponentID, &msg, &D);

		LOG_I("<- cmdSetMessageTarget id = " + i2str((int)id) + ", interval = " + i2str((int)interval) + ", responseTarget = " + i2str((int)responseTarget));
		return writeMessage(msg);
	}

	void _Mavlink::sendSetMsgInterval(void)
	{
		for (MavMsgBase *pM : m_vpMsg)
		{
			IF_CONT(pM->m_tInterval < 0);
			IF_CONT(pM->bReceiving());
			clSetMessageInterval(pM->m_id, pM->m_tInterval, 0);
		}
	}

	bool _Mavlink::setMsgInterval(int id, uint64_t tInt)
	{
		for (MavMsgBase *pM : m_vpMsg)
		{
			IF_CONT(pM->m_id != id);

			pM->m_tInterval = tInt;
			return true;
		}

		return false;
	}

	void _Mavlink::console(void *pConsole)
	{
		this->_ModuleBase::console(pConsole);
		IF_(check() != OK_OK);

		_Console *pC = (_Console *)pConsole;
		if (!m_pIO->bOpen())
		{
			pC->addMsg("Not Connected", 0);
			return;
		}

		pC->addMsg("Connected", 0);
		pC->addMsg("mySysID = " + i2str(m_mySystemID) + " myComID = " + i2str(m_myComponentID) + " myType = " + i2str(m_myType));
		pC->addMsg("devSysID = " + i2str(m_devSystemID) + " devComID = " + i2str(m_devComponentID) + " devType = " + i2str(m_devType));
		pC->addMsg("Dropped packets = " + i2str(m_status.packet_rx_drop_count));
		pC->addMsg("Mission seq=" + i2str(m_missionCurrent.m_msg.seq) + " tot=" + i2str(m_missionCurrent.m_msg.total) + " mode = " + i2str(m_missionCurrent.m_msg.mission_mode) + " state = " + i2str(m_missionCurrent.m_msg.mission_state));
	}

}
