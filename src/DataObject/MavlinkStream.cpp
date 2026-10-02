/*
 * MavlinkStream.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavlinkStream.h"

namespace kai
{

	MavlinkStream::MavlinkStream()
	{
		m_vpMsgRegistry.push_back(&m_attitude);
		m_vpMsgRegistry.push_back(&m_attitudeQuaternion);
		m_vpMsgRegistry.push_back(&m_batteryStatus);
		m_vpMsgRegistry.push_back(&m_commandAck);
		m_vpMsgRegistry.push_back(&m_cmdInt);
		m_vpMsgRegistry.push_back(&m_cmdLong);
		m_vpMsgRegistry.push_back(&m_distanceSensor);
		m_vpMsgRegistry.push_back(&m_globalPositionINT);
		m_vpMsgRegistry.push_back(&m_globalVisionPositionEstimate);
		m_vpMsgRegistry.push_back(&m_gpsInput);
		m_vpMsgRegistry.push_back(&m_gpsRawINT);
		m_vpMsgRegistry.push_back(&m_gpsRTCMdata);
		m_vpMsgRegistry.push_back(&m_heartbeat);
		m_vpMsgRegistry.push_back(&m_highresIMU);
		m_vpMsgRegistry.push_back(&m_homePosition);
		m_vpMsgRegistry.push_back(&m_landingTarget);
		m_vpMsgRegistry.push_back(&m_localPositionNED);

		m_vpMsgRegistry.push_back(&m_missionAck);
		m_vpMsgRegistry.push_back(&m_missionClearAll);
		m_vpMsgRegistry.push_back(&m_missionCount);
		m_vpMsgRegistry.push_back(&m_missionCurrent);
		m_vpMsgRegistry.push_back(&m_missionItemInt);
		m_vpMsgRegistry.push_back(&m_missionItemReached);
		m_vpMsgRegistry.push_back(&m_missionRequestInt);
		m_vpMsgRegistry.push_back(&m_missionRequestList);
		m_vpMsgRegistry.push_back(&m_missionSetCurrent);

		m_vpMsgRegistry.push_back(&m_mountConfigure);
		m_vpMsgRegistry.push_back(&m_mountControl);
		m_vpMsgRegistry.push_back(&m_mountStatus);
		m_vpMsgRegistry.push_back(&m_paramRequestRead);
		m_vpMsgRegistry.push_back(&m_paramSet);
		m_vpMsgRegistry.push_back(&m_paramValue);
		m_vpMsgRegistry.push_back(&m_positionTargetLocalNED);
		m_vpMsgRegistry.push_back(&m_positionTargetGlobalINT);
		m_vpMsgRegistry.push_back(&m_radioStatus);
		m_vpMsgRegistry.push_back(&m_rawIMU);
		m_vpMsgRegistry.push_back(&m_rcChannels);
		m_vpMsgRegistry.push_back(&m_rcChannelsOverride);
		m_vpMsgRegistry.push_back(&m_requestDataStream);
		m_vpMsgRegistry.push_back(&m_servoOutputRaw);
		m_vpMsgRegistry.push_back(&m_setAttitudeTarget);
		m_vpMsgRegistry.push_back(&m_setMode);
		m_vpMsgRegistry.push_back(&m_setPositionTargetLocalNED);
		m_vpMsgRegistry.push_back(&m_setPositionTargetGlobalINT);
		m_vpMsgRegistry.push_back(&m_statusText);
		m_vpMsgRegistry.push_back(&m_sysStatus);
		m_vpMsgRegistry.push_back(&m_scaledIMU);
		m_vpMsgRegistry.push_back(&m_visionPositionEstimate);
		m_vpMsgRegistry.push_back(&m_visionSpeedEstimate);
	}

	MavlinkStream::~MavlinkStream()
	{
	}

	bool MavlinkStream::loadConfig(void)
	{
		IF_F(!this->DataObjBase::loadConfig());
		json &j = *m_pJ;

		jKv(j, "nMsgQueue", m_nMsgQueue);

		clearMsgQueue(m_nMsgQueue);

		return true;
	}

	bool MavlinkStream::saveConfig(bool bExport)
	{
		IF_F(!this->DataObjBase::saveConfig(false));
		{
			std::shared_lock lock(m_sMutex);
			json &j = *m_pJ;
			j["nMsgQueue"] = m_nMsgQueue;
		}

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool MavlinkStream::decode(const mavlink_message_t &msg)
	{
		for (MavMsgBase *pM : m_vpMsgRegistry)
		{
			IF_CONT(pM->getID() != msg.msgid);

			pM->decode(msg); // decode and call callbacks
			LOG_I("Decoded MSG_ID: " + i2str(msg.msgid));
			return true;
		}

		LOG_I("Unknown MSG_ID: " + i2str(msg.msgid));
		return false;
	}

	//TODO
	void MavlinkStream::addMsgQueue(const MAV_MSG_TSTAMP& m)
	{
		std::unique_lock lock(m_sMutex);

		m_vMsgQueue.push_back(m);
	}

	void MavlinkStream::getMsgQueue(vector<MAV_MSG_TSTAMP> &vMsg, uint64_t tStampFrom)
	{
		std::shared_lock lock(m_sMutex);

		vMsg.clear();
		vMsg.reserve(m_vMsgQueue.size());

		for (const auto &m : m_vMsgQueue)
		{
			if (m.m_tStamp > tStampFrom)
				vMsg.push_back(m);
		}
	}

	void MavlinkStream::clearMsgQueue(size_t nMsg)
	{
		std::unique_lock lock(m_sMutex);
		m_vMsgQueue.clear();

		if(nMsg > 0)
		{
			m_nMsgQueue = nMsg;
			m_vMsgQueue.reserve(m_nMsgQueue);
		}
	}

	void MavlinkStream::sendSetMsgInterval(MavlinkStream *pOut)
	{
		NULL_(pOut);
		for (MavMsgBase *pM : m_vpMsgRegistry)
		{
			IF_CONT(pM->getDesiredInterval() < 0); // desired interval not specified
			IF_CONT(pM->bOnTime());

			// MAV_CMD_SET_MESSAGE_INTERVAL uses microseconds on the wire.
			pOut->clSetMessageInterval(pM->getID(), ((float)pM->getDesiredInterval()) * USEC_NSEC, 0);
		}
	}

	bool MavlinkStream::setMsgInterval(int id, int64_t tIntNsec)
	{
		IF_F(id < 0);
		uint32_t msgID = static_cast<uint32_t>(id);

		for (MavMsgBase *pM : m_vpMsgRegistry)
		{
			IF_CONT(pM->getID() != msgID);

			pM->setDesiredInterval(tIntNsec);
			return true;
		}

		return false;
	}


	void MavlinkStream::attitude(mavlink_attitude_t &D)
	{
		MAV_MSG_TSTAMP m;
		m.m_msg = m_attitude.encode(D, );
		addMsgQueue(&m_attitude, D);

		LOG_I("setTargetAttitude");
	}

	void MavlinkStream::attitudeQuaternion(mavlink_attitude_quaternion_t &D)
	{
		addMsgQueue(&m_attitudeQuaternion, D);
		LOG_I("attitudeQuaternion");
	}

	void MavlinkStream::batteryStatus(mavlink_battery_status_t &D)
	{
		addMsgQueue(&m_batteryStatus, D);
		LOG_I("batteryStatus");
	}

	void MavlinkStream::commandAck(mavlink_command_ack_t &D)
	{
		addMsgQueue(&m_commandAck, D);
		LOG_I("commandAck");
	}

	void MavlinkStream::cmdInt(mavlink_command_int_t &D)
	{
		addMsgQueue(&m_cmdInt, D);
		LOG_I("CMD_INT command = " + i2str(D.command));
	}

	void MavlinkStream::cmdLong(mavlink_command_long_t &D)
	{
		addMsgQueue(&m_cmdLong, D);
		LOG_I("CMD_LONG command = " + i2str(D.command));
	}

	void MavlinkStream::distanceSensor(mavlink_distance_sensor_t &D)
	{
		addMsgQueue(&m_distanceSensor, D);
		LOG_I(
			"DIST_SENSOR orient = " + i2str((int)D.orientation) + ", d = " + i2str((int)D.current_distance) + ", min = " + i2str((int)D.min_distance) + ", max = " + i2str((int)D.max_distance));
	}

	void MavlinkStream::globalPositionInt(mavlink_global_position_int_t &D)
	{
		addMsgQueue(&m_globalPositionINT, D);
		LOG_I(
			"GLOBAL_POS_INT lat=" + i2str(D.lat) + ", lon=" + i2str(D.lon) + ", alt=" + i2str(D.alt) + ", relative_alt=" + i2str(D.relative_alt) + ", vx=" + i2str(D.vx) + ", vy=" + i2str(D.vy) + ", vz=" + i2str(D.vz) + ", hdg=" + i2str(D.hdg));
	}

	void MavlinkStream::globalVisionPositionEstimate(
		mavlink_global_vision_position_estimate_t &D)
	{
		addMsgQueue(&m_globalVisionPositionEstimate, D);
		LOG_I(
			"GLOBAL_VISION_POSITION_ESTIMATE T=" + i2str(D.usec) + ", x=" + f2str(D.x) + ", y=" + f2str(D.y) + ", z=" + f2str(D.z) + "; roll=" + f2str(D.roll) + ", pitch=" + f2str(D.pitch) + ", yaw=" + f2str(D.yaw));
	}

	void MavlinkStream::gpsInput(mavlink_gps_input_t &D)
	{
		addMsgQueue(&m_gpsInput, D);
		LOG_I(
			"GPS_INPUT lat=" + i2str(D.lat) + ", lon=" + i2str(D.lon) + ", alt=" + f2str(D.alt));
	}

	void MavlinkStream::gpsRawINT(mavlink_gps_raw_int_t &D)
	{
		addMsgQueue(&m_gpsRawINT, D);
		LOG_I("GPS_RAW_INT lat=" + i2str(D.lat) + ", lon=" + i2str(D.lon) + ", alt=" + i2str(D.alt) + ", fix_type=" + i2str(D.fix_type));
	}

	void MavlinkStream::gpsRTCMdata(mavlink_gps_rtcm_data_t &D)
	{
		addMsgQueue(&m_gpsRTCMdata, D);
		LOG_I("GPS_RTCM_DATA len=" + i2str(D.len));
	}

	void MavlinkStream::highresIMU(mavlink_highres_imu_t &D)
	{
		addMsgQueue(&m_highresIMU, D);
		LOG_I("highresIMU");
	}

	void MavlinkStream::homePosition(mavlink_home_position_t &D)
	{
		addMsgQueue(&m_homePosition, D);
		LOG_I("homePosition");
	}

	void MavlinkStream::landingTarget(mavlink_landing_target_t &D)
	{
		addMsgQueue(&m_landingTarget, D);
		LOG_I(
			"landingTarget: angleX=" + f2str(D.angle_x) + ", angleY=" + f2str(D.angle_y) + ", distance=" + f2str(D.distance));
	}

	void MavlinkStream::localPositionNED(mavlink_local_position_ned_t &D)
	{
		addMsgQueue(&m_localPositionNED, D);
		LOG_I("localPositionNED");
	}

	void MavlinkStream::missionAck(mavlink_mission_ack_t &D)
	{
		addMsgQueue(&m_missionAck, D);
		LOG_I(
			"missionAck: type=" + i2str(D.mission_type) + ", opaqueID=" + i2str(D.opaque_id));
	}

	void MavlinkStream::missionClearAll(mavlink_mission_clear_all_t &D)
	{
		addMsgQueue(&m_missionClearAll, D);
		LOG_I(
			"missionClearAll: type=" + i2str(D.mission_type));
	}

	void MavlinkStream::missionCount(mavlink_mission_count_t &D)
	{
		addMsgQueue(&m_missionCount, D);
		LOG_I(
			"missionCount: count=" + i2str(D.count) + ", type=" + i2str(D.mission_type) + ", opaqueID=" + i2str(D.opaque_id));
	}

	void MavlinkStream::missionCurrent(mavlink_mission_current_t &D)
	{
		addMsgQueue(&m_missionCurrent, D);
		LOG_I(
			"missionCurrent: missionID=" + i2str(D.mission_id));
	}

	void MavlinkStream::missionItemInt(mavlink_mission_item_int_t &D)
	{
		addMsgQueue(&m_missionItemInt, D);
		LOG_I(
			"missionItemInt");
	}

	void MavlinkStream::missionItemReached(mavlink_mission_item_reached_t &D)
	{
		addMsgQueue(&m_missionItemReached, D);
		LOG_I(
			"missionItemReached: seq=" + i2str(D.seq));
	}

	void MavlinkStream::missionRequestInt(mavlink_mission_request_int_t &D)
	{
		addMsgQueue(&m_missionRequestInt, D);
		LOG_I(
			"missionRequestInt: seq=" + i2str(D.seq) + ", type=" + i2str(D.mission_type));
	}

	void MavlinkStream::missionRequestList(mavlink_mission_request_list_t &D)
	{
		addMsgQueue(&m_missionRequestList, D);
		LOG_I(
			"missionRequestList: type=" + i2str(D.mission_type));
	}

	void MavlinkStream::missionSetCurrent(mavlink_mission_set_current_t &D)
	{
		addMsgQueue(&m_missionSetCurrent, D);
		LOG_I(
			"missionSetCurrent: seq=" + i2str(D.seq));
	}

	void MavlinkStream::mountConfigure(mavlink_mount_configure_t &D)
	{
		addMsgQueue(&m_mountConfigure, D);
		LOG_I(
			"mountConfigure: roll=" + i2str(D.stab_roll) + ", pitch=" + i2str(D.stab_pitch) + ", yaw=" + i2str(D.stab_yaw) + ", mode=" + i2str(D.mount_mode));
	}

	void MavlinkStream::mountControl(mavlink_mount_control_t &D)
	{
		addMsgQueue(&m_mountControl, D);
		LOG_I(
			"mountControl: pitch=" + i2str(D.input_a) + ", roll=" + i2str(D.input_b) + ", yaw=" + i2str(D.input_c) + ", savePos=" + i2str(D.save_position));
	}

	void MavlinkStream::mountStatus(mavlink_mount_status_t &D)
	{
		addMsgQueue(&m_mountStatus, D);
		LOG_I(
			"mountStatus: a=" + i2str(D.pointing_a) + ", b=" + i2str(D.pointing_b) + ", c=" + i2str(D.pointing_c));
	}

	void MavlinkStream::paramRequestRead(mavlink_param_request_read_t &D)
	{
		addMsgQueue(&m_paramRequestRead, D);
		if (m_bLog)
		{
			char id[17];
			memcpy(id, D.param_id, 16);
			id[16] = 0;

			LOG_I(
				"paramRequestRead: id=" + string(D.param_id) + ", index=" + i2str((int)D.param_index));
		}
	}

	void MavlinkStream::paramSet(mavlink_param_set_t &D)
	{
		addMsgQueue(&m_paramSet, D);
		if (m_bLog)
		{
			char id[17];
			memcpy(id, D.param_id, 16);
			id[16] = 0;

			LOG_I(
				"paramSet: type=" + i2str(D.param_type) + ", value=" + f2str(D.param_value) + ", id=" + string(id));
		}
	}

	void MavlinkStream::paramValue(mavlink_param_value_t &D)
	{
		addMsgQueue(&m_paramValue, D);
		if (m_bLog)
		{
			char id[17];
			memcpy(id, D.param_id, 16);
			id[16] = 0;

			LOG_I(
				"paramValue: type=" + i2str(D.param_type) + ", value=" + f2str(D.param_value) + ", id=" + string(id));
		}
	}

	void MavlinkStream::positionTargetLocalNed(mavlink_position_target_local_ned_t &D)
	{
		addMsgQueue(&m_positionTargetLocalNED, D);
		LOG_I(
			"POS_TARGET_LOCAL_NED x=" + f2str(D.x) + ", y=" + f2str(D.y) + ", z=" + f2str(D.z) + ", vx=" + f2str(D.vx) + ", vy=" + f2str(D.vy) + ", vz=" + f2str(D.vz) + ", afx=" + f2str(D.afx) + ", afy=" + f2str(D.afy) + ", afz=" + f2str(D.afz) + ", yaw=" + f2str(D.yaw) + ", yawRate=" + f2str(D.yaw_rate));
	}

	void MavlinkStream::positionTargetGlobalInt(mavlink_position_target_global_int_t &D)
	{
		addMsgQueue(&m_positionTargetGlobalINT, D);
		LOG_I(
			"POS_TARGET_GLOBAL_INT lat=" + i2str(D.lat_int) + ", lng=" + i2str(D.lon_int) + ", alt=" + f2str(D.alt) + ", vx=" + f2str(D.vx) + ", vy=" + f2str(D.vy) + ", vz=" + f2str(D.vz) + ", afx=" + f2str(D.afx) + ", afy=" + f2str(D.afy) + ", afz=" + f2str(D.afz) + ", yaw=" + f2str(D.yaw) + ", yawRate=" + f2str(D.yaw_rate));
	}

	void MavlinkStream::radioStatus(mavlink_radio_status_t &D)
	{
		addMsgQueue(&m_radioStatus, D);
		LOG_I("radioStatus");
	}

	void MavlinkStream::rawIMU(mavlink_raw_imu_t &D)
	{
		addMsgQueue(&m_rawIMU, D);
		LOG_I("rawIMU");
	}

	void MavlinkStream::rcChannels(mavlink_rc_channels_t &D)
	{
		addMsgQueue(&m_rcChannels, D);
		LOG_I("rcChannels");
	}

	void MavlinkStream::rcChannelsOverride(mavlink_rc_channels_override_t &D)
	{
		addMsgQueue(&m_rcChannelsOverride, D);
		LOG_I(
			"rcChannelsOverride, c1=" + i2str(D.chan1_raw) + ", c2=" + i2str(D.chan2_raw) + ", c3=" + i2str(D.chan3_raw) + ", c4=" + i2str(D.chan4_raw) + ", c5=" + i2str(D.chan5_raw) + ", c6=" + i2str(D.chan6_raw) + ", c7=" + i2str(D.chan7_raw) + ", c8=" + i2str(D.chan8_raw));
	}

	void MavlinkStream::requestDataStream(mavlink_request_data_stream_t &D)
	{
		addMsgQueue(&m_requestDataStream, D);
		LOG_I("requestDataStream");
	}

	void MavlinkStream::requestDataStream(uint8_t stream_id, int rate)
	{
		mavlink_request_data_stream_t D{};
		D.req_stream_id = stream_id;
		D.req_message_rate = rate;
		D.start_stop = 1;

		addMsgQueue(&m_requestDataStream, D);

		LOG_I("requestDataStream");
	}

	void MavlinkStream::heartbeat(mavlink_heartbeat_t &D)
	{
		addMsgQueue(&m_heartbeat, D);
		LOG_I("heartBeat");
	}

	void MavlinkStream::servoOutputRaw(mavlink_servo_output_raw_t &D)
	{
		addMsgQueue(&m_servoOutputRaw, D);
		LOG_I("servoOutputRaw");
	}

	void MavlinkStream::setAttitudeTarget(mavlink_set_attitude_target_t &D)
	{
		addMsgQueue(&m_setAttitudeTarget, D);
		LOG_I("setTargetAttitude");
	}

	void MavlinkStream::setMode(mavlink_set_mode_t &D)
	{
		addMsgQueue(&m_setMode, D);
		LOG_I(
			"setMode, base_mode=" + i2str((int32_t)D.base_mode) + ", custom_mode=" + i2str(D.custom_mode));
	}

	void MavlinkStream::setPositionTargetLocalNED(
		mavlink_set_position_target_local_ned_t &D)
	{
		addMsgQueue(&m_setPositionTargetLocalNED, D);
		LOG_I(
			"setPositionTargetLocalNED, x=" + f2str(D.x) + ", y=" + f2str(D.y) + ", z=" + f2str(D.z) + ", vx=" + f2str(D.vx) + ", vy=" + f2str(D.vy) + ", vz=" + f2str(D.vz));
	}

	void MavlinkStream::setPositionTargetGlobalINT(
		mavlink_set_position_target_global_int_t &D)
	{
		addMsgQueue(&m_setPositionTargetGlobalINT, D);
		LOG_I(
			"setPositionTargetGlobalINT, lat=" + i2str(D.lat_int) + ", lon=" + i2str(D.lon_int) + ", alt=" + f2str(D.alt) + ", vx=" + f2str(D.vx) + ", vy=" + f2str(D.vy) + ", vz=" + f2str(D.vz));
	}

	void MavlinkStream::statusText(mavlink_statustext_t &D)
	{
		addMsgQueue(&m_statusText, D);
		LOG_I(
			"statusText: " + string(D.text));
	}

	void MavlinkStream::sysStatus(mavlink_sys_status_t &D)
	{
		addMsgQueue(&m_sysStatus, D);
		LOG_I("sysStatus");
	}

	void MavlinkStream::scaledIMU(mavlink_scaled_imu_t &D)
	{
		addMsgQueue(&m_scaledIMU, D);
		LOG_I("scaledIMU");
	}

	void MavlinkStream::visionPositionEstimate(mavlink_vision_position_estimate_t &D)
	{
		addMsgQueue(&m_visionPositionEstimate, D);
		LOG_I(
			"VISION_POSITION_ESTIMATE T=" + i2str(D.usec) + ", x=" + f2str(D.x) + ", y=" + f2str(D.y) + ", z=" + f2str(D.z) + "; roll=" + f2str(D.roll) + ", pitch=" + f2str(D.pitch) + ", yaw=" + f2str(D.yaw));
	}

	void MavlinkStream::visionSpeedEstimate(mavlink_vision_speed_estimate_t &D)
	{
		addMsgQueue(&m_visionSpeedEstimate, D);
		LOG_I(
			"VISION_SPEED_ESTIMATE T=" + i2str(D.usec) + ", x=" + f2str(D.x) + ", y=" + f2str(D.y) + ", z=" + f2str(D.z));
	}

	// CMD_LONG

	void MavlinkStream::clComponentArmDisarm(bool bArm)
	{
		mavlink_command_long_t D{};
		D.command = MAV_CMD_COMPONENT_ARM_DISARM;
		D.confirmation = 0;
		D.param1 = (bArm) ? 1 : 0;
		D.param2 = 0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		addMsgQueue(&m_cmdLong, D);

		LOG_I("cmdLongComponentArmDisarm: " + i2str(bArm));
	}

	void MavlinkStream::clDoFlightTermination(bool bTerminate)
	{
		mavlink_command_long_t D{};
		D.command = MAV_CMD_DO_FLIGHTTERMINATION;
		D.confirmation = 0;
		D.param1 = (bTerminate) ? 1 : 0;
		D.param2 = 0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		addMsgQueue(&m_cmdLong, D);

		LOG_I("cmdLongFlightTermination: " + i2str(bTerminate));
	}

	void MavlinkStream::clDoSetMode(int mode)
	{
		mavlink_command_long_t D{};
		D.command = MAV_CMD_DO_SET_MODE;
		D.confirmation = 0;
		D.param1 = mode;
		D.param2 = 0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		addMsgQueue(&m_cmdLong, D);

		LOG_I("cmdLongDoSetMode: " + i2str(mode));
	}

	void MavlinkStream::clNavSetYawSpeed(float yaw, float speed, float yawMode)
	{
		mavlink_command_long_t D{};
		D.command = MAV_CMD_NAV_SET_YAW_SPEED;
		D.confirmation = 0;
		D.param1 = yaw;
		D.param2 = speed;
		D.param3 = yawMode;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		addMsgQueue(&m_cmdLong, D);

		LOG_I("cmdLongDoSetPositionYawTrust: yaw=" + f2str(yaw) + ", speed=" + f2str(speed) + ", yawMode=" + f2str(yawMode));
	}

	void MavlinkStream::clDoSetServo(int iServo, int PWM)
	{
		mavlink_command_long_t D{};
		D.command = MAV_CMD_DO_SET_SERVO;
		D.confirmation = 0;
		D.param1 = iServo;
		D.param2 = (float)PWM;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		addMsgQueue(&m_cmdLong, D);

		LOG_I(
			"cmdLongDoSetServo: servo=" + i2str((int)iServo) + " pwm=" + i2str(PWM));
	}

	void MavlinkStream::clDoSetRelay(int iRelay, bool bRelay)
	{
		mavlink_command_long_t D{};
		D.command = MAV_CMD_DO_SET_RELAY;
		D.confirmation = 0;
		D.param1 = iRelay;
		D.param2 = (bRelay) ? 1.0 : 0.0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		addMsgQueue(&m_cmdLong, D);

		LOG_I(
			"cmdLongDoSetRelay: relay=" + i2str((int)iRelay) + " relay=" + i2str((int)bRelay));
	}

	void MavlinkStream::clDoSetHome(bool bUseCurrent, float r, float p, float y, float lat, float lon, float alt)
	{
		mavlink_command_long_t D{};
		D.command = MAV_CMD_DO_SET_HOME;
		D.confirmation = 0;
		D.param1 = (bUseCurrent) ? 1 : 0;
		D.param2 = r;
		D.param3 = p;
		D.param4 = y;
		D.param5 = lat;
		D.param6 = lon;
		D.param7 = alt;

		addMsgQueue(&m_cmdLong, D);

		LOG_I("cmdLongSetHome");
	}

	void MavlinkStream::clGetHomePosition(void)
	{
		mavlink_command_long_t D{};
		D.command = MAV_CMD_GET_HOME_POSITION;
		D.confirmation = 0;
		D.param1 = 0;
		D.param2 = 0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		addMsgQueue(&m_cmdLong, D);

		LOG_I("cmdLongGetHomePosition");
	}

	void MavlinkStream::clNavTakeoff(float alt)
	{
		mavlink_command_long_t D{};
		D.command = MAV_CMD_NAV_TAKEOFF;
		D.confirmation = 0;
		D.param1 = 0;
		D.param2 = 0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = alt;

		addMsgQueue(&m_cmdLong, D);

		LOG_I("cmdNavTakeoff");
	}

	void MavlinkStream::clNavRTL(void)
	{
		mavlink_command_long_t D{};
		D.command = MAV_CMD_NAV_RETURN_TO_LAUNCH;
		D.confirmation = 0;
		D.param1 = 0;
		D.param2 = 0;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = 0;

		addMsgQueue(&m_cmdLong, D);

		LOG_I("cmdNavRTL");
	}

	void MavlinkStream::clSetMessageInterval(float id, float interval, float responseTarget)
	{
		mavlink_command_long_t D{};
		D.command = MAV_CMD_SET_MESSAGE_INTERVAL;
		D.confirmation = 0;
		D.param1 = id;
		D.param2 = interval;
		D.param3 = 0;
		D.param4 = 0;
		D.param5 = 0;
		D.param6 = 0;
		D.param7 = responseTarget;

		addMsgQueue(&m_cmdLong, D);

		LOG_I("cmdSetMessageTarget id = " + i2str((int)id) + ", interval = " + i2str((int)interval) + ", responseTarget = " + i2str((int)responseTarget));
	}

	void MavlinkStream::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjBase::console(pConsole);
	}

}
