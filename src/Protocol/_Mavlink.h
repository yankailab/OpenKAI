#ifndef OpenKAI_src_Protocol__Mavlink_H_
#define OpenKAI_src_Protocol__Mavlink_H_

#include "../Base/common.h"
#include "../Dependency/mavlink/ardupilotmega/mavlink.h"
#include "../Dependency/mavlink/mavlink_conversions.h"
#include "../IO/_IOBase.h"
#include "../IO/_WebSocket.h"

#define MAV_N_PEER 16
#define MAV_N_CMD_U64 4
#define MAV_N_CMD 256

namespace kai
{

class MavMsgBase
{
public:
	MavMsgBase()
	{
		m_id = 0x7fffffff;
		m_tStamp = 0;
		m_tInterval = -1;
	};

	virtual ~MavMsgBase(void){};

	void setRecvRate(int64_t tInterval)
	{
		m_tInterval = tInterval;
	}

	bool bReceiving(void)
	{
		IF_T(m_tInterval < 0);
		IF_T(m_tStamp > 0);
		return false;
	}

	virtual void decode(mavlink_message_t* pM)
	{
	}

	uint32_t	m_id;
	uint64_t	m_tStamp;
	int64_t		m_tInterval;
};

class MavAttitude : public MavMsgBase
{
public:
	mavlink_attitude_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_ATTITUDE;

		m_msg.yaw = 0;
		m_msg.yawspeed = 0;
		m_msg.pitch = 0;
		m_msg.pitchspeed = 0;
		m_msg.roll = 0;
		m_msg.rollspeed = 0;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_attitude_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavBatteryStatus : public MavMsgBase
{
public:
	mavlink_battery_status_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_BATTERY_STATUS;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_battery_status_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavCommandAck : public MavMsgBase
{
public:
	mavlink_command_ack_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_COMMAND_ACK;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_command_ack_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavGlobalPositionINT : public MavMsgBase
{
public:
	mavlink_global_position_int_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_GLOBAL_POSITION_INT;

		m_msg.alt = 0;
		m_msg.lat = 0.0;
		m_msg.lon = 0.0;
		m_msg.relative_alt = 0;
		m_msg.hdg = UINT16_MAX;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_global_position_int_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavHeartbeat : public MavMsgBase
{
public:
	mavlink_heartbeat_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_HEARTBEAT;

		m_msg.custom_mode = 0;
		m_msg.base_mode = 0;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_heartbeat_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavHighresIMU : public MavMsgBase
{
public:
	mavlink_highres_imu_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_HIGHRES_IMU;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_highres_imu_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavHomePosition : public MavMsgBase
{
public:
	mavlink_home_position_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_HOME_POSITION;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_home_position_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavLocalPositionNED : public MavMsgBase
{
public:
	mavlink_local_position_ned_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_LOCAL_POSITION_NED;

		m_msg.vx = 0;
		m_msg.vy = 0;
		m_msg.vz = 0;
		m_msg.x = 0;
		m_msg.y = 0;
		m_msg.z = 0;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_local_position_ned_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavMissionCurrent : public MavMsgBase
{
public:
	mavlink_mission_current_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_MISSION_CURRENT;

		m_msg.seq = 0;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_mission_current_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavMountStatus : public MavMsgBase
{
public:
	mavlink_mount_status_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_MOUNT_STATUS;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_mount_status_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavParamSet : public MavMsgBase
{
public:
	mavlink_param_set_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_PARAM_SET;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_param_set_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavPositionTargetLocalNED : public MavMsgBase
{
public:
	mavlink_position_target_local_ned_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_position_target_local_ned_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavPositionTargetGlobalINT : public MavMsgBase
{
public:
	mavlink_position_target_global_int_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_position_target_global_int_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavRadioStatus : public MavMsgBase
{
public:
	mavlink_radio_status_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_RADIO_STATUS;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_radio_status_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavRawIMU : public MavMsgBase
{
public:
	mavlink_raw_imu_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_RAW_IMU;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_raw_imu_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavRcChannels : public MavMsgBase
{
public:
	mavlink_rc_channels_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_RC_CHANNELS;

		m_msg.chancount = 0;
		m_msg.chan1_raw = UINT16_MAX;
		m_msg.chan2_raw = UINT16_MAX;
		m_msg.chan3_raw = UINT16_MAX;
		m_msg.chan4_raw = UINT16_MAX;
		m_msg.chan5_raw = UINT16_MAX;
		m_msg.chan6_raw = UINT16_MAX;
		m_msg.chan7_raw = UINT16_MAX;
		m_msg.chan8_raw = UINT16_MAX;
		m_msg.chan9_raw = UINT16_MAX;
		m_msg.chan10_raw = UINT16_MAX;
		m_msg.chan11_raw = UINT16_MAX;
		m_msg.chan12_raw = UINT16_MAX;
		m_msg.chan13_raw = UINT16_MAX;
		m_msg.chan14_raw = UINT16_MAX;
		m_msg.chan15_raw = UINT16_MAX;
		m_msg.chan16_raw = UINT16_MAX;
		m_msg.chan17_raw = UINT16_MAX;
		m_msg.chan18_raw = UINT16_MAX;
		m_msg.rssi = 255;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_rc_channels_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavRcChannelsOverride : public MavMsgBase
{
public:
	mavlink_rc_channels_override_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_rc_channels_override_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavSysStatus : public MavMsgBase
{
public:
	mavlink_sys_status_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_SYS_STATUS;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_sys_status_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};

class MavScaledIMU : public MavMsgBase
{
public:
	mavlink_scaled_imu_t m_msg;

	void init(void)
	{
		m_id = MAVLINK_MSG_ID_SCALED_IMU;
	}

	void decode(mavlink_message_t* pM)
	{
		mavlink_msg_scaled_imu_decode(pM, &m_msg);
		m_tStamp = getTimeUsec();
	}
};


struct MAVLINK_PEER
{
	void *m_pPeer;
	uint64_t m_pCmdRoute[MAV_N_CMD_U64]; //index of bit corresponds to Mavlink CMD ID

	void init(void)
	{
		m_pPeer = NULL;
		reset();
	}

	void reset(void)
	{
		for (int i = 0; i < MAV_N_CMD_U64; i++)
		{
			m_pCmdRoute[i] = 0xffffffffffffffff;
		}
	}

	bool bCmdRoute(uint32_t iCmd)
	{
		IF_F(iCmd >= MAV_N_CMD);
		return m_pCmdRoute[iCmd / 64] & (1 << (iCmd % 64));
	}

	void setCmdRoute(uint32_t iCmd, bool bON)
	{
		IF_(iCmd >= MAV_N_CMD);

		if (bON)
			m_pCmdRoute[iCmd / 64] |= (1 << (iCmd % 64));
		else
			m_pCmdRoute[iCmd / 64] &= ~(1 << (iCmd % 64));
	}
};

class _Mavlink: public _ThreadBase
{
public:
	_Mavlink();
	~_Mavlink();

	bool init(void *pKiss);
	bool start(void);
	void draw(void);

	//Receive
	void handleMessages();
	bool readMessage(mavlink_message_t &message);

	//Send
	void writeMessage(mavlink_message_t message);

	//Cmd
	void cmdInt(mavlink_command_int_t &D);
	void distanceSensor(mavlink_distance_sensor_t &D);
	void globalVisionPositionEstimate(mavlink_global_vision_position_estimate_t &D);
	void gpsInput(mavlink_gps_input_t &D);
	void globalPositionInt(mavlink_global_position_int_t &D);
	void landingTarget(mavlink_landing_target_t &D);
	void mountConfigure(mavlink_mount_configure_t &D);
	void mountControl(mavlink_mount_control_t &D);
	void mountStatus(mavlink_mount_status_t &D);
	void param_set(mavlink_param_set_t &D);
	void positionTargetLocalNed(mavlink_position_target_local_ned_t &D);
	void positionTargetGlobalInt(mavlink_position_target_global_int_t &D);
	void rcChannelsOverride(mavlink_rc_channels_override_t &D);
	void requestDataStream(uint8_t stream_id, int rate);
	void heartbeat(void);
	void setAttitudeTarget(mavlink_set_attitude_target_t &D);
	void setMode(mavlink_set_mode_t &D);
	void setPositionTargetLocalNED(mavlink_set_position_target_local_ned_t &D);
	void setPositionTargetGlobalINT(mavlink_set_position_target_global_int_t &D);
	void visionPositionEstimate(mavlink_vision_position_estimate_t &D);

	//Cmd long
	void clComponentArmDisarm(bool bArm);
	void clDoSetMode(int mode);
	void clDoSetServo(int iServo, int PWM);
	void clDoSetRelay(int iRelay, bool bRelay);
	void clGetHomePosition(void);
	void clNavSetYawSpeed(float yaw, float speed, float yawMode);
	void clNavTakeoff(float alt);
	void clSetMessageInterval(float id, float interval, float responseTarget);

	//Msg routing
	void setCmdRoute(uint32_t iCmd, bool bON);

	//Utility
	uint16_t* getRCinRaw(int iChan);
	void sendSetMsgInterval(void);
	bool setMsgInterval(int id, int tInt);


private:
	void update(void);
	static void* getUpdateThread(void *This)
	{
		((_Mavlink*) This)->update();
		return NULL;
	}

public:
	mavlink_status_t m_status;
	vector<MAVLINK_PEER> m_vPeer;

	MavAttitude m_attitude;
	MavBatteryStatus m_batteryStatus;
	MavCommandAck m_commandAck;
	MavGlobalPositionINT m_globalPositionINT;
	MavHeartbeat m_heartbeat;
	MavHighresIMU m_highresIMU;
	MavHomePosition m_homePosition;
	MavLocalPositionNED m_localPositionNED;
	MavMissionCurrent m_missionCurrent;
	MavMountStatus m_mountStatus;
	MavParamSet m_paramSet;
	MavPositionTargetLocalNED m_positionTargetLocalNED;
	MavPositionTargetGlobalINT m_positionTargetGlobalINT;
	MavRadioStatus m_radioStatus;
	MavRawIMU m_rawIMU;
	MavRcChannels m_rcChannels;
	MavRcChannelsOverride m_rcChannelsOverride;
	MavSysStatus m_sysStatus;
	MavScaledIMU m_scaledIMU;
	vector<MavMsgBase*> m_vpMsg;

private:
	_IOBase *m_pIO;
	int m_mySystemID;
	int m_myComponentID;
	int m_myType;
	int m_devSystemID;
	int m_devComponentID;
	int m_devType;

	uint8_t m_rBuf[N_IO_BUF];
	int m_nRead;
	int m_iRead;
};

}

#endif
