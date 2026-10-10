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
		clearMsgQueue();
	}

	MavlinkStream::~MavlinkStream()
	{
	}

	bool MavlinkStream::loadConfig(void)
	{
		IF_F(!this->DataObjBase::loadConfig());
		json &j = *m_pJ;

		size_t nMsgQueue = m_nMsgQueue;
		jKv(j, "nMsgQueue", nMsgQueue);
		IF_Le_F(nMsgQueue == 0, "Invalid MAVLink message queue capacity");
		clearMsgQueue(nMsgQueue);

		return true;
	}

	bool MavlinkStream::saveConfig(bool bExport)
	{
		IF_F(!this->DataObjBase::saveConfig(false));
		{
			std::shared_lock lock(m_sMutexMq);
			json &j = *m_pJ;
			j["nMsgQueue"] = m_nMsgQueue;
		}

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool MavlinkStream::decode(const mavlink_message_t &msg)
	{
		std::lock_guard<std::recursive_mutex> lock(m_receiveMutex);
		for (MavMsgBase *pM : m_vpMsgRegistry)
		{
			IF_CONT(pM->getID() != msg.msgid);

			// Deliver each segment synchronously; callers assemble fragmented messages.
			pM->decode(msg);

			LOG_I("Decoded MSG_ID: " + i2str(msg.msgid));
			return true;
		}

		LOG_I("Unknown MSG_ID: " + i2str(msg.msgid));
		return false;
	}

	void MavlinkStream::sendSetMsgInterval()
	{
		for (MavMsgBase *pM : m_vpMsgRegistry)
		{
			IF_CONT(pM->getDesiredInterval() < 0); // desired interval not specified
			IF_CONT(pM->bOnTime());

			// MAV_CMD_SET_MESSAGE_INTERVAL uses microseconds on the wire.
			clSetMessageInterval(pM->getID(), ((float)pM->getDesiredInterval()) * USEC_NSEC, 0);
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

	void MavlinkStream::clearMsgQueue(size_t nMb)
	{
		std::unique_lock lock(m_sMutexMq);
		m_vMsgQueue.clear();
		m_iMqSet = 0;

		if (nMb > 0)
		{
			m_nMsgQueue = nMb;
		}
		m_vMsgQueue.resize(m_nMsgQueue);
		// Preserve m_tLastQueued so existing readers survive a clear or resize.
	}

	void MavlinkStream::addMsgQueueLocked(const MAV_MSG_TSTAMP &mT)
	{
		MAV_MSG_TSTAMP &queued = m_vMsgQueue[m_iMqSet];

		queued.m_msgT = mT.m_msgT;
		queued.m_tStamp = std::max(getTns(), m_tLastQueued + 1);
		m_tLastQueued = queued.m_tStamp;

		if (++m_iMqSet == m_vMsgQueue.size())
			m_iMqSet = 0;
	}

	uint64_t MavlinkStream::getEncodedMsgs(vector<mavlink_message_t> &vMsg, uint64_t tStampFrom)
	{
		std::shared_lock lock(m_sMutexMq);

		vMsg.clear();
		uint64_t tLatest = tStampFrom;
		IF__(m_tLastQueued <= tStampFrom, tLatest);
		size_t iMsg = m_iMqSet;
		for (size_t n = 0; n < m_vMsgQueue.size(); ++n)
		{
			const MAV_MSG_TSTAMP &mT = m_vMsgQueue[iMsg];
			if (mT.m_tStamp > tStampFrom)
			{
				vMsg.push_back(mT.m_msgT);
				tLatest = mT.m_tStamp;
			}
			if (++iMsg == m_vMsgQueue.size())
				iMsg = 0;
		}

		return tLatest;
	}

	// CMD_LONG
	void MavlinkStream::clComponentArmDisarm(bool bArm, uint8_t mySysID, uint8_t myComID)
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

		add<MavCommandLong>(D, mySysID, myComID);

		LOG_I("cmdLongComponentArmDisarm: " + i2str(bArm));
	}

	void MavlinkStream::clDoFlightTermination(bool bTerminate, uint8_t mySysID, uint8_t myComID)
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

		add<MavCommandLong>(D, mySysID, myComID);

		LOG_I("cmdLongFlightTermination: " + i2str(bTerminate));
	}

	void MavlinkStream::clDoSetMode(int mode, uint8_t mySysID, uint8_t myComID)
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

		add<MavCommandLong>(D, mySysID, myComID);

		LOG_I("cmdLongDoSetMode: " + i2str(mode));
	}

	void MavlinkStream::clNavSetYawSpeed(float yaw, float speed, float yawMode, uint8_t mySysID, uint8_t myComID)
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

		add<MavCommandLong>(D, mySysID, myComID);

		LOG_I("cmdLongDoSetPositionYawTrust: yaw=" + f2str(yaw) + ", speed=" + f2str(speed) + ", yawMode=" + f2str(yawMode));
	}

	void MavlinkStream::clDoSetServo(int iServo, int PWM, uint8_t mySysID, uint8_t myComID)
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

		add<MavCommandLong>(D, mySysID, myComID);

		LOG_I(
			"cmdLongDoSetServo: servo=" + i2str((int)iServo) + " pwm=" + i2str(PWM));
	}

	void MavlinkStream::clDoSetRelay(int iRelay, bool bRelay, uint8_t mySysID, uint8_t myComID)
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

		add<MavCommandLong>(D, mySysID, myComID);

		LOG_I(
			"cmdLongDoSetRelay: relay=" + i2str((int)iRelay) + " relay=" + i2str((int)bRelay));
	}

	void MavlinkStream::clDoSetHome(bool bUseCurrent, float r, float p, float y, float lat, float lon, float alt, uint8_t mySysID, uint8_t myComID)
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

		add<MavCommandLong>(D, mySysID, myComID);

		LOG_I("cmdLongSetHome");
	}

	void MavlinkStream::clGetHomePosition(uint8_t mySysID, uint8_t myComID)
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

		add<MavCommandLong>(D, mySysID, myComID);

		LOG_I("cmdLongGetHomePosition");
	}

	void MavlinkStream::clNavTakeoff(float alt, uint8_t mySysID, uint8_t myComID)
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

		add<MavCommandLong>(D, mySysID, myComID);

		LOG_I("cmdNavTakeoff");
	}

	void MavlinkStream::clNavRTL(uint8_t mySysID, uint8_t myComID)
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

		add<MavCommandLong>(D, mySysID, myComID);

		LOG_I("cmdNavRTL");
	}

	void MavlinkStream::clSetMessageInterval(float id, float interval, float responseTarget, uint8_t mySysID, uint8_t myComID)
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

		add<MavCommandLong>(D, mySysID, myComID);

		LOG_I("cmdSetMessageTarget id = " + i2str((int)id) + ", interval = " + i2str((int)interval) + ", responseTarget = " + i2str((int)responseTarget));
	}

	void MavlinkStream::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjBase::console(pConsole);
	}

}
