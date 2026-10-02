/*
 * MavlinkStream.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__MavlinkStream__H_
#define OpenKAI_src__DataStream__MavlinkStream__H_

#include "DataObjBase.h"
#include "Mavlink/MavMsgBase.h"
#include "Mavlink/MavAttitude.h"
#include "Mavlink/MavAttitudeQuaternion.h"
#include "Mavlink/MavBatteryStatus.h"
#include "Mavlink/MavCommandAck.h"
#include "Mavlink/MavCommandInt.h"
#include "Mavlink/MavCommandLong.h"
#include "Mavlink/MavDistanceSensor.h"
#include "Mavlink/MavGlobalPositionINT.h"
#include "Mavlink/MavGlobalVisionPositionEstimate.h"
#include "Mavlink/MavGpsInput.h"
#include "Mavlink/MavGpsRawINT.h"
#include "Mavlink/MavGpsRTCMdata.h"
#include "Mavlink/MavHeartbeat.h"
#include "Mavlink/MavHighresIMU.h"
#include "Mavlink/MavHomePosition.h"
#include "Mavlink/MavLandingTarget.h"
#include "Mavlink/MavLocalPositionNED.h"
#include "Mavlink/MavMissionCount.h"
#include "Mavlink/MavMissionCurrent.h"
#include "Mavlink/MavMissionRequestList.h"
#include "Mavlink/MavMissionRequestInt.h"
#include "Mavlink/MavMissionItemInt.h"
#include "Mavlink/MavMissionAck.h"
#include "Mavlink/MavMissionSetCurrent.h"
#include "Mavlink/MavMissionClearAll.h"
#include "Mavlink/MavMissionItemReached.h"
#include "Mavlink/MavMountConfigure.h"
#include "Mavlink/MavMountControl.h"
#include "Mavlink/MavMountStatus.h"
#include "Mavlink/MavParamRequestRead.h"
#include "Mavlink/MavParamSet.h"
#include "Mavlink/MavParamValue.h"
#include "Mavlink/MavPositionTargetLocalNED.h"
#include "Mavlink/MavPositionTargetGlobalINT.h"
#include "Mavlink/MavRadioStatus.h"
#include "Mavlink/MavRawIMU.h"
#include "Mavlink/MavRcChannels.h"
#include "Mavlink/MavRcChannelsOverride.h"
#include "Mavlink/MavRequestDataStream.h"
#include "Mavlink/MavServoOutputRaw.h"
#include "Mavlink/MavSetAttitudeTarget.h"
#include "Mavlink/MavSetMode.h"
#include "Mavlink/MavSetPositionTargetLocalNED.h"
#include "Mavlink/MavSetPositionTargetGlobalINT.h"
#include "Mavlink/MavStatusText.h"
#include "Mavlink/MavSysStatus.h"
#include "Mavlink/MavScaledIMU.h"
#include "Mavlink/MavVisionPositionEstimate.h"
#include "Mavlink/MavVisionSpeedEstimate.h"
#include <deque>
#include <memory>
#include <mutex>

namespace kai
{
	class MavlinkStream : public DataObjBase
	{
	public:
		MavlinkStream();
		virtual ~MavlinkStream();
		bool loadConfig(void);
		bool saveConfig(bool bExport = false);
		void console(void *pConsole) override;

		// caller add a msg received from IO
		bool decode(const mavlink_message_t &msg);
		// set desired update rate for message
		void sendSetMsgInterval(void);
		bool setMsgInterval(int id, int64_t tInt);


		uint64_t getMsgQueue(vector<mavlink_message_t> &vMsg, uint64_t tStampFrom = 0);
	protected:
		// caller added msg into a queue to be written to IO
		void clearMsgQueue(size_t nMb = 0);
		void addMsgQueue(MavMsgBase *pMb);

		std::shared_mutex m_sMutexMq;
		std::vector<MavMsgBase *> m_vMsgQueue;	// ring buffer. message from each class pointer will be sent as a batch, not necessary to preserve the send order across different classes
		size_t m_nMsgQueue = 1024;
		size_t m_iMqSet = 0;
		uint64_t m_tLastQueued = 0;


		vector<MavMsgBase *> m_vpMsgRegistry;
	public:
		// caller call these functions send a msg to IO.
		// each message class has its own internal queue, for Mavlink message with fragmented sequences just call the same setter multiple times
		void attitude(mavlink_attitude_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void attitudeQuaternion(mavlink_attitude_quaternion_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void batteryStatus(mavlink_battery_status_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void commandAck(mavlink_command_ack_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void cmdInt(mavlink_command_int_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void cmdLong(mavlink_command_long_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void distanceSensor(mavlink_distance_sensor_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void globalPositionInt(mavlink_global_position_int_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void globalVisionPositionEstimate(mavlink_global_vision_position_estimate_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void gpsInput(mavlink_gps_input_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void gpsRawINT(mavlink_gps_raw_int_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void gpsRTCMdata(mavlink_gps_rtcm_data_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void highresIMU(mavlink_highres_imu_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void homePosition(mavlink_home_position_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void landingTarget(mavlink_landing_target_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void localPositionNED(mavlink_local_position_ned_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);

		void missionAck(mavlink_mission_ack_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void missionClearAll(mavlink_mission_clear_all_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void missionCount(mavlink_mission_count_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void missionCurrent(mavlink_mission_current_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void missionItemInt(mavlink_mission_item_int_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void missionItemReached(mavlink_mission_item_reached_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void missionRequestInt(mavlink_mission_request_int_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void missionRequestList(mavlink_mission_request_list_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void missionSetCurrent(mavlink_mission_set_current_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);

		void mountConfigure(mavlink_mount_configure_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void mountControl(mavlink_mount_control_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void mountStatus(mavlink_mount_status_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void paramRequestRead(mavlink_param_request_read_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void paramSet(mavlink_param_set_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void paramValue(mavlink_param_value_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void positionTargetLocalNed(mavlink_position_target_local_ned_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void positionTargetGlobalInt(mavlink_position_target_global_int_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void radioStatus(mavlink_radio_status_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void rawIMU(mavlink_raw_imu_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void rcChannels(mavlink_rc_channels_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void rcChannelsOverride(mavlink_rc_channels_override_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void requestDataStream(mavlink_request_data_stream_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void requestDataStream(uint8_t stream_id, int rate);
		void heartbeat(mavlink_heartbeat_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void servoOutputRaw(mavlink_servo_output_raw_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void setAttitudeTarget(mavlink_set_attitude_target_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void setMode(mavlink_set_mode_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void setPositionTargetLocalNED(mavlink_set_position_target_local_ned_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void setPositionTargetGlobalINT(mavlink_set_position_target_global_int_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void statusText(mavlink_statustext_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void sysStatus(mavlink_sys_status_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void scaledIMU(mavlink_scaled_imu_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void visionPositionEstimate(mavlink_vision_position_estimate_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);
		void visionSpeedEstimate(mavlink_vision_speed_estimate_t &D, uint8_t mySysID = 0, uint8_t myComID = 0);

		// Cmd long
		void clComponentArmDisarm(bool bArm, uint8_t mySysID = 0, uint8_t myComID = 0);
		void clDoFlightTermination(bool bTerminate, uint8_t mySysID = 0, uint8_t myComID = 0);
		void clDoSetMode(int mode, uint8_t mySysID = 0, uint8_t myComID = 0);
		void clDoSetServo(int iServo, int PWM, uint8_t mySysID = 0, uint8_t myComID = 0);
		void clDoSetRelay(int iRelay, bool bRelay, uint8_t mySysID = 0, uint8_t myComID = 0);
		void clDoSetHome(bool bUseCurrent, float r, float p, float y, float lat, float lon, float alt, uint8_t mySysID = 0, uint8_t myComID = 0);
		void clGetHomePosition(uint8_t mySysID = 0, uint8_t myComID = 0);
		void clNavSetYawSpeed(float yaw, float speed, float yawMode, uint8_t mySysID = 0, uint8_t myComID = 0);
		void clNavTakeoff(float alt, uint8_t mySysID = 0, uint8_t myComID = 0);
		void clNavRTL(uint8_t mySysID = 0, uint8_t myComID = 0);
		void clSetMessageInterval(float id, float interval, float responseTarget, uint8_t mySysID = 0, uint8_t myComID = 0);

		MavAttitude m_attitude;
		MavAttitudeQuaternion m_attitudeQuaternion;
		MavBatteryStatus m_batteryStatus;
		MavCommandAck m_commandAck;
		MavCommandInt m_cmdInt;
		MavCommandLong m_cmdLong;
		MavDistanceSensor m_distanceSensor;
		MavGlobalPositionINT m_globalPositionINT;
		MavGlobalVisionPositionEstimate m_globalVisionPositionEstimate;
		MavGpsInput m_gpsInput;
		MavGpsRawINT m_gpsRawINT;
		MavGpsRTCMdata m_gpsRTCMdata;
		MavHeartbeat m_heartbeat;
		MavHighresIMU m_highresIMU;
		MavHomePosition m_homePosition;
		MavLandingTarget m_landingTarget;
		MavLocalPositionNED m_localPositionNED;

		MavMissionAck m_missionAck;
		MavMissionClearAll m_missionClearAll;
		MavMissionCount m_missionCount;
		MavMissionCurrent m_missionCurrent;
		MavMissionItemInt m_missionItemInt;
		MavMissionItemReached m_missionItemReached;
		MavMissionRequestInt m_missionRequestInt;
		MavMissionRequestList m_missionRequestList;
		MavMissionSetCurrent m_missionSetCurrent;

		MavMountConfigure m_mountConfigure;
		MavMountControl m_mountControl;
		MavMountStatus m_mountStatus;
		MavParamRequestRead m_paramRequestRead;
		MavParamSet m_paramSet;
		MavParamValue m_paramValue;
		MavPositionTargetLocalNED m_positionTargetLocalNED;
		MavPositionTargetGlobalINT m_positionTargetGlobalINT;
		MavRadioStatus m_radioStatus;
		MavRawIMU m_rawIMU;
		MavRcChannels m_rcChannels;
		MavRcChannelsOverride m_rcChannelsOverride;
		MavRequestDataStream m_requestDataStream;
		MavServoOutputRaw m_servoOutputRaw;
		MavSetAttitudeTarget m_setAttitudeTarget;
		MavSetMode m_setMode;
		MavSetPositionTargetLocalNED m_setPositionTargetLocalNED;
		MavSetPositionTargetGlobalINT m_setPositionTargetGlobalINT;
		MavStatusText m_statusText;
		MavSysStatus m_sysStatus;
		MavScaledIMU m_scaledIMU;
		MavVisionPositionEstimate m_visionPositionEstimate;
		MavVisionSpeedEstimate m_visionSpeedEstimate;
	};

}
#endif
