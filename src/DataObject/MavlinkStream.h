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
#include <type_traits>

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

		// Receive from IO
		bool decode(const mavlink_message_t &msg);

		// Copy received telemetry and its timestamp together while decode is excluded.
		// Polling consumers must use this instead of retaining a message's get() reference.
		template <typename T, typename M>
		bool snapshot(M &message, uint64_t &timestamp)
		{
			std::lock_guard<std::recursive_mutex> lock(m_receiveMutex);
			T *source = get<T>();
			timestamp = source->getTstamp();
			if (!timestamp) return false;
			message = source->get();
			return true;
		}

		// Send to IO
		// Each call queues one encoded message; callers split fragmented sequences.
		template <typename T, typename M>
		void add(M msg, uint8_t mySysID = 0, uint8_t myComID = 0)
		{
			std::unique_lock lock(m_sMutexMq);
			T* pT = get<T>();
			addMsgQueueLocked(pT->add(msg, mySysID, myComID));
		}

		// helpers for command long
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

		// set desired update rate for message
		void sendSetMsgInterval(void);
		bool setMsgInterval(int id, int64_t tInt);

		// _Mavlink instance call this to get encoded message queue and send to IO
		uint64_t getEncodedMsgs(vector<mavlink_message_t> &vMsg, uint64_t tStampFrom = 0);

		template <typename T>
		T *get() noexcept
		{
			if constexpr (std::is_same_v<T, MavAttitude>)
				return &m_attitude;
			else if constexpr (std::is_same_v<T, MavAttitudeQuaternion>)
				return &m_attitudeQuaternion;
			else if constexpr (std::is_same_v<T, MavBatteryStatus>)
				return &m_batteryStatus;
			else if constexpr (std::is_same_v<T, MavCommandAck>)
				return &m_commandAck;
			else if constexpr (std::is_same_v<T, MavCommandInt>)
				return &m_cmdInt;
			else if constexpr (std::is_same_v<T, MavCommandLong>)
				return &m_cmdLong;
			else if constexpr (std::is_same_v<T, MavDistanceSensor>)
				return &m_distanceSensor;
			else if constexpr (std::is_same_v<T, MavGlobalPositionINT>)
				return &m_globalPositionINT;
			else if constexpr (std::is_same_v<T, MavGlobalVisionPositionEstimate>)
				return &m_globalVisionPositionEstimate;
			else if constexpr (std::is_same_v<T, MavGpsInput>)
				return &m_gpsInput;
			else if constexpr (std::is_same_v<T, MavGpsRawINT>)
				return &m_gpsRawINT;
			else if constexpr (std::is_same_v<T, MavGpsRTCMdata>)
				return &m_gpsRTCMdata;
			else if constexpr (std::is_same_v<T, MavHeartbeat>)
				return &m_heartbeat;
			else if constexpr (std::is_same_v<T, MavHighresIMU>)
				return &m_highresIMU;
			else if constexpr (std::is_same_v<T, MavHomePosition>)
				return &m_homePosition;
			else if constexpr (std::is_same_v<T, MavLandingTarget>)
				return &m_landingTarget;
			else if constexpr (std::is_same_v<T, MavLocalPositionNED>)
				return &m_localPositionNED;
			else if constexpr (std::is_same_v<T, MavMissionAck>)
				return &m_missionAck;
			else if constexpr (std::is_same_v<T, MavMissionClearAll>)
				return &m_missionClearAll;
			else if constexpr (std::is_same_v<T, MavMissionCount>)
				return &m_missionCount;
			else if constexpr (std::is_same_v<T, MavMissionCurrent>)
				return &m_missionCurrent;
			else if constexpr (std::is_same_v<T, MavMissionItemInt>)
				return &m_missionItemInt;
			else if constexpr (std::is_same_v<T, MavMissionItemReached>)
				return &m_missionItemReached;
			else if constexpr (std::is_same_v<T, MavMissionRequestInt>)
				return &m_missionRequestInt;
			else if constexpr (std::is_same_v<T, MavMissionRequestList>)
				return &m_missionRequestList;
			else if constexpr (std::is_same_v<T, MavMissionSetCurrent>)
				return &m_missionSetCurrent;
			else if constexpr (std::is_same_v<T, MavMountConfigure>)
				return &m_mountConfigure;
			else if constexpr (std::is_same_v<T, MavMountControl>)
				return &m_mountControl;
			else if constexpr (std::is_same_v<T, MavMountStatus>)
				return &m_mountStatus;
			else if constexpr (std::is_same_v<T, MavParamRequestRead>)
				return &m_paramRequestRead;
			else if constexpr (std::is_same_v<T, MavParamSet>)
				return &m_paramSet;
			else if constexpr (std::is_same_v<T, MavParamValue>)
				return &m_paramValue;
			else if constexpr (std::is_same_v<T, MavPositionTargetLocalNED>)
				return &m_positionTargetLocalNED;
			else if constexpr (std::is_same_v<T, MavPositionTargetGlobalINT>)
				return &m_positionTargetGlobalINT;
			else if constexpr (std::is_same_v<T, MavRadioStatus>)
				return &m_radioStatus;
			else if constexpr (std::is_same_v<T, MavRawIMU>)
				return &m_rawIMU;
			else if constexpr (std::is_same_v<T, MavRcChannels>)
				return &m_rcChannels;
			else if constexpr (std::is_same_v<T, MavRcChannelsOverride>)
				return &m_rcChannelsOverride;
			else if constexpr (std::is_same_v<T, MavRequestDataStream>)
				return &m_requestDataStream;
			else if constexpr (std::is_same_v<T, MavServoOutputRaw>)
				return &m_servoOutputRaw;
			else if constexpr (std::is_same_v<T, MavSetAttitudeTarget>)
				return &m_setAttitudeTarget;
			else if constexpr (std::is_same_v<T, MavSetMode>)
				return &m_setMode;
			else if constexpr (std::is_same_v<T, MavSetPositionTargetLocalNED>)
				return &m_setPositionTargetLocalNED;
			else if constexpr (std::is_same_v<T, MavSetPositionTargetGlobalINT>)
				return &m_setPositionTargetGlobalINT;
			else if constexpr (std::is_same_v<T, MavStatusText>)
				return &m_statusText;
			else if constexpr (std::is_same_v<T, MavSysStatus>)
				return &m_sysStatus;
			else if constexpr (std::is_same_v<T, MavScaledIMU>)
				return &m_scaledIMU;
			else if constexpr (std::is_same_v<T, MavVisionPositionEstimate>)
				return &m_visionPositionEstimate;
			else if constexpr (std::is_same_v<T, MavVisionSpeedEstimate>)
				return &m_visionSpeedEstimate;
			else
				static_assert(!std::is_same_v<T, T>, "Unsupported MAVLink message type");
		}

	protected:
		std::recursive_mutex m_receiveMutex;
		void clearMsgQueue(size_t nMb = 0);

		std::shared_mutex m_sMutexMq;
		std::vector<MAV_MSG_TSTAMP> m_vMsgQueue; // Retains the newest m_nMsgQueue messages in enqueue order.
		size_t m_nMsgQueue = 1024;
		size_t m_iMqSet = 0;
		uint64_t m_tLastQueued = 0;

		// Message object instances
		vector<MavMsgBase *> m_vpMsgRegistry;

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

	private:
		// Caller holds m_sMutexMq from encoding through publication.
		void addMsgQueueLocked(const MAV_MSG_TSTAMP& mT);
	};
}
#endif
