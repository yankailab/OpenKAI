#ifndef OpenKAI_src_Protocol__Mavlink_H_
#define OpenKAI_src_Protocol__Mavlink_H_

#include "../Dependencies/c_library_v2/ardupilotmega/mavlink.h"
#include "../Dependencies/c_library_v2/mavlink_conversions.h"
#include "../IO/_IObase.h"
#include "../IO/_WebSocket.h"

#define MAV_N_PEER 16
#define MAV_N_CMD_U64 4
#define MAV_N_CMD 256
#define MAV_N_BUF 512

namespace kai
{
	typedef void (*CbMavMsg)(void *pMsg, void *pInst);
	struct MavCallback
	{
		CbMavMsg m_pCbRecv = NULL;
		void *m_pCbInst = NULL;

		void callback(void* pMavMsg)
		{
			NULL_(m_pCbRecv);
			m_pCbRecv(pMavMsg, m_pCbInst);
		}
	};

	class MavMsgBase
	{
	public:
		MavMsgBase()
		{
			m_id = 0x7fffffff;
			m_tStamp = 0;
			m_tInterval = -1;
		};

		virtual ~MavMsgBase(void) {};

		void setRecvRate(uint64_t tInterval)
		{
			m_tInterval = tInterval;
		}

		bool bReceiving(uint64_t tNow)
		{
			IF__(m_tStamp >= tNow, true);
			IF__(tNow - m_tStamp < m_tInterval + 1000, true);

			return false;
		}

		virtual void decode(mavlink_message_t *pM)
		{
			m_tStamp = getTbootUs();

			for (MavCallback c : m_vCbRecv)
			{
				c.callback(this);
			}
		}

		bool addCbRecv(CbMavMsg pCb, void *pInst)
		{
			NULL_F(pCb);

			for (MavCallback c : m_vCbRecv)
			{
				if((c.m_pCbRecv == pCb) && (c.m_pCbInst == pInst))
					return true;
			}

			MavCallback cb;
			cb.m_pCbRecv = pCb;
			cb.m_pCbInst = pInst;
			m_vCbRecv.push_back(cb);

			return true;
		}

		void clearCbRecv(CbMavMsg pCb, void *pInst)
		{
			NULL_(pCb);

			for (int i = 0; i < m_vCbRecv.size(); i++)
			{
				MavCallback *pC = &m_vCbRecv[i];
				IF_CONT((pC->m_pCbRecv != pCb) || (pC->m_pCbInst != pInst));

				vector<MavCallback>::iterator it = m_vCbRecv.begin() + i;
				m_vCbRecv.erase(it);

				return;
			}
		}

		void clearAllCbRecv(void)
		{
			m_vCbRecv.clear();
		}

	public:
		uint32_t m_id;
		uint64_t m_tStamp;
		int64_t m_tInterval;

		vector<MavCallback> m_vCbRecv;
	};

	class MavAttitude : public MavMsgBase
	{
	public:
		mavlink_attitude_t m_msg;

		MavAttitude()
		{
			m_id = MAVLINK_MSG_ID_ATTITUDE;

			m_msg.yaw = 0;
			m_msg.yawspeed = 0;
			m_msg.pitch = 0;
			m_msg.pitchspeed = 0;
			m_msg.roll = 0;
			m_msg.rollspeed = 0;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_attitude_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavAttitudeQuaternion : public MavMsgBase
	{
	public:
		mavlink_attitude_quaternion_t m_msg;

		MavAttitudeQuaternion()
		{
			m_id = MAVLINK_MSG_ID_ATTITUDE_QUATERNION;

			m_msg.q1 = 0;
			m_msg.q2 = 0;
			m_msg.q3 = 0;
			m_msg.q4 = 0;
			m_msg.rollspeed = 0;
			m_msg.pitchspeed = 0;
			m_msg.yawspeed = 0;
			m_msg.repr_offset_q[0] = 0;
			m_msg.repr_offset_q[1] = 0;
			m_msg.repr_offset_q[2] = 0;
			m_msg.repr_offset_q[3] = 0;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_attitude_quaternion_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavBatteryStatus : public MavMsgBase
	{
	public:
		mavlink_battery_status_t m_msg;

		MavBatteryStatus()
		{
			m_id = MAVLINK_MSG_ID_BATTERY_STATUS;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_battery_status_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavCommandAck : public MavMsgBase
	{
	public:
		mavlink_command_ack_t m_msg;

		MavCommandAck()
		{
			m_id = MAVLINK_MSG_ID_COMMAND_ACK;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_command_ack_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavGlobalPositionINT : public MavMsgBase
	{
	public:
		mavlink_global_position_int_t m_msg;

		MavGlobalPositionINT()
		{
			m_id = MAVLINK_MSG_ID_GLOBAL_POSITION_INT;

			m_msg.alt = 0;
			m_msg.lat = 0.0;
			m_msg.lon = 0.0;
			m_msg.relative_alt = 0;
			m_msg.hdg = UINT16_MAX;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_global_position_int_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavGpsRawINT : public MavMsgBase
	{
	public:
		mavlink_gps_raw_int_t m_msg;

		MavGpsRawINT()
		{
			m_id = MAVLINK_MSG_ID_GPS_RAW_INT;

			m_msg.fix_type = GPS_FIX_TYPE_NO_GPS;
			m_msg.alt = 0;
			m_msg.lat = 0.0;
			m_msg.lon = 0.0;
			m_msg.h_acc = UINT32_MAX;
			m_msg.v_acc = UINT32_MAX;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_gps_raw_int_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavHeartbeat : public MavMsgBase
	{
	public:
		mavlink_heartbeat_t m_msg;

		MavHeartbeat()
		{
			m_id = MAVLINK_MSG_ID_HEARTBEAT;

			m_msg.custom_mode = 0;
			m_msg.base_mode = 0;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_heartbeat_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavHighresIMU : public MavMsgBase
	{
	public:
		mavlink_highres_imu_t m_msg;

		MavHighresIMU()
		{
			m_id = MAVLINK_MSG_ID_HIGHRES_IMU;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_highres_imu_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavHomePosition : public MavMsgBase
	{
	public:
		mavlink_home_position_t m_msg;

		MavHomePosition()
		{
			m_id = MAVLINK_MSG_ID_HOME_POSITION;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_home_position_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavLocalPositionNED : public MavMsgBase
	{
	public:
		mavlink_local_position_ned_t m_msg;

		MavLocalPositionNED()
		{
			m_id = MAVLINK_MSG_ID_LOCAL_POSITION_NED;

			m_msg.vx = 0;
			m_msg.vy = 0;
			m_msg.vz = 0;
			m_msg.x = 0;
			m_msg.y = 0;
			m_msg.z = 0;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_local_position_ned_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavMissionCount : public MavMsgBase
	{
	public:
		mavlink_mission_count_t m_msg;

		MavMissionCount()
		{
			m_id = MAVLINK_MSG_ID_MISSION_COUNT;

			m_msg.count = 0;
			m_msg.mission_type = 0;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_mission_count_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavMissionCurrent : public MavMsgBase
	{
	public:
		mavlink_mission_current_t m_msg;

		MavMissionCurrent()
		{
			m_id = MAVLINK_MSG_ID_MISSION_CURRENT;

			m_msg.seq = 0;
			m_msg.total = 0;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_mission_current_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavMissionRequestList : public MavMsgBase
	{
	public:
		mavlink_mission_request_list_t m_msg;

		MavMissionRequestList()
		{
			m_id = MAVLINK_MSG_ID_MISSION_REQUEST_LIST;

			m_msg.mission_type = 0;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_mission_request_list_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavMissionRequestInt : public MavMsgBase
	{
	public:
		mavlink_mission_request_int_t m_msg;

		MavMissionRequestInt()
		{
			m_id = MAVLINK_MSG_ID_MISSION_REQUEST_INT;

			m_msg.mission_type = 0;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_mission_request_int_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavMissionItemInt : public MavMsgBase
	{
	public:
		mavlink_mission_item_int_t m_msg;

		MavMissionItemInt()
		{
			m_id = MAVLINK_MSG_ID_MISSION_ITEM_INT;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_mission_item_int_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavMissionAck : public MavMsgBase
	{
	public:
		mavlink_mission_ack_t m_msg;

		MavMissionAck()
		{
			m_id = MAVLINK_MSG_ID_MISSION_ACK;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_mission_ack_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavMissionSetCurrent : public MavMsgBase
	{
	public:
		mavlink_mission_set_current_t m_msg;

		MavMissionSetCurrent()
		{
			m_id = MAVLINK_MSG_ID_MISSION_SET_CURRENT;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_mission_set_current_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavMissionClearAll : public MavMsgBase
	{
	public:
		mavlink_mission_clear_all_t m_msg;

		MavMissionClearAll()
		{
			m_id = MAVLINK_MSG_ID_MISSION_CLEAR_ALL;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_mission_clear_all_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavMissionItemReached : public MavMsgBase
	{
	public:
		mavlink_mission_item_reached_t m_msg;

		MavMissionItemReached()
		{
			m_id = MAVLINK_MSG_ID_MISSION_ITEM_REACHED;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_mission_item_reached_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavMountStatus : public MavMsgBase
	{
	public:
		mavlink_mount_status_t m_msg;

		MavMountStatus()
		{
			m_id = MAVLINK_MSG_ID_MOUNT_STATUS;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_mount_status_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavParamRequestRead : public MavMsgBase
	{
	public:
		mavlink_param_request_read_t m_msg;

		MavParamRequestRead()
		{
			m_id = MAVLINK_MSG_ID_PARAM_REQUEST_READ;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_param_request_read_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavParamSet : public MavMsgBase
	{
	public:
		mavlink_param_set_t m_msg;

		MavParamSet()
		{
			m_id = MAVLINK_MSG_ID_PARAM_SET;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_param_set_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavParamValue : public MavMsgBase
	{
	public:
		mavlink_param_value_t m_msg;

		MavParamValue()
		{
			m_id = MAVLINK_MSG_ID_PARAM_VALUE;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_param_value_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavPositionTargetLocalNED : public MavMsgBase
	{
	public:
		mavlink_position_target_local_ned_t m_msg;

		MavPositionTargetLocalNED()
		{
			m_id = MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_position_target_local_ned_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavPositionTargetGlobalINT : public MavMsgBase
	{
	public:
		mavlink_position_target_global_int_t m_msg;

		MavPositionTargetGlobalINT()
		{
			m_id = MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_position_target_global_int_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavRadioStatus : public MavMsgBase
	{
	public:
		mavlink_radio_status_t m_msg;

		MavRadioStatus()
		{
			m_id = MAVLINK_MSG_ID_RADIO_STATUS;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_radio_status_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavRawIMU : public MavMsgBase
	{
	public:
		mavlink_raw_imu_t m_msg;

		MavRawIMU()
		{
			m_id = MAVLINK_MSG_ID_RAW_IMU;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_raw_imu_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavRcChannels : public MavMsgBase
	{
	public:
		mavlink_rc_channels_t m_msg;
		uint16_t *m_pChan[19];

		MavRcChannels()
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

			m_pChan[0] = NULL;
			m_pChan[1] = &m_msg.chan1_raw;
			m_pChan[2] = &m_msg.chan2_raw;
			m_pChan[3] = &m_msg.chan3_raw;
			m_pChan[4] = &m_msg.chan4_raw;
			m_pChan[5] = &m_msg.chan5_raw;
			m_pChan[6] = &m_msg.chan6_raw;
			m_pChan[7] = &m_msg.chan7_raw;
			m_pChan[8] = &m_msg.chan8_raw;
			m_pChan[9] = &m_msg.chan9_raw;
			m_pChan[10] = &m_msg.chan10_raw;
			m_pChan[11] = &m_msg.chan11_raw;
			m_pChan[12] = &m_msg.chan12_raw;
			m_pChan[13] = &m_msg.chan13_raw;
			m_pChan[14] = &m_msg.chan14_raw;
			m_pChan[15] = &m_msg.chan15_raw;
			m_pChan[16] = &m_msg.chan16_raw;
			m_pChan[17] = &m_msg.chan17_raw;
			m_pChan[18] = &m_msg.chan18_raw;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_rc_channels_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}

		uint16_t getRC(int iChan)
		{
			if (iChan <= 0 || iChan > 18)
				return UINT16_MAX;

			return *m_pChan[iChan];
		}
	};

	class MavRcChannelsOverride : public MavMsgBase
	{
	public:
		mavlink_rc_channels_override_t m_msg;

		MavRcChannelsOverride()
		{
			m_id = MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_rc_channels_override_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavStatusText : public MavMsgBase
	{
	public:
		mavlink_statustext_t m_msg;

		MavStatusText()
		{
			m_id = MAVLINK_MSG_ID_STATUSTEXT;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_statustext_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavSysStatus : public MavMsgBase
	{
	public:
		mavlink_sys_status_t m_msg;

		MavSysStatus()
		{
			m_id = MAVLINK_MSG_ID_SYS_STATUS;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_sys_status_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	class MavScaledIMU : public MavMsgBase
	{
	public:
		mavlink_scaled_imu_t m_msg;

		MavScaledIMU()
		{
			m_id = MAVLINK_MSG_ID_SCALED_IMU;
		}

		void decode(mavlink_message_t *pM)
		{
			mavlink_msg_scaled_imu_decode(pM, &m_msg);
			this->MavMsgBase::decode(pM);
		}
	};

	struct MAVLINK_PEER
	{
		void *m_pPeer;
		uint64_t m_pCmdRoute[MAV_N_CMD_U64]; // index of bit corresponds to Mavlink CMD ID

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

	class _Mavlink : public _ModuleBase
	{
	public:
		_Mavlink();
		~_Mavlink();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual void console(void *pConsole);

		bool bConnected(void);

		// Receive
		bool readMessage(mavlink_message_t &message);

		// Send
		void writeMessage(mavlink_message_t message);

		// Cmd
		void cmdInt(mavlink_command_int_t &D);
		void distanceSensor(mavlink_distance_sensor_t &D);
		void globalVisionPositionEstimate(mavlink_global_vision_position_estimate_t &D);
		void gpsInput(mavlink_gps_input_t &D);
		void globalPositionInt(mavlink_global_position_int_t &D);
		void gpsRawINT(mavlink_gps_raw_int_t &D);
		void landingTarget(mavlink_landing_target_t &D);

		void missionAck(mavlink_mission_ack_t &D);
		void missionClearAll(mavlink_mission_clear_all_t &D);
		void missionCount(mavlink_mission_count_t &D);
		void missionCurrent(mavlink_mission_current_t &D);
		void missionItemInt(mavlink_mission_item_int_t &D);
		void missionItemReached(mavlink_mission_item_reached_t &D);
		void missionRequestInt(mavlink_mission_request_int_t &D);
		void missionRequestList(mavlink_mission_request_list_t &D);
		void missionSetCurrent(mavlink_mission_set_current_t &D);

		void mountConfigure(mavlink_mount_configure_t &D);
		void mountControl(mavlink_mount_control_t &D);
		void mountStatus(mavlink_mount_status_t &D);
		void paramRequestRead(mavlink_param_request_read_t &D);
		void paramSet(mavlink_param_set_t &D);
		void paramValue(mavlink_param_value_t &D);
		void positionTargetLocalNed(mavlink_position_target_local_ned_t &D);
		void positionTargetGlobalInt(mavlink_position_target_global_int_t &D);
		void rcChannelsOverride(mavlink_rc_channels_override_t &D);
		void requestDataStream(uint8_t stream_id, int rate);
		void heartbeat(void);
		void setAttitudeTarget(mavlink_set_attitude_target_t &D);
		void setMode(mavlink_set_mode_t &D);
		void setPositionTargetLocalNED(mavlink_set_position_target_local_ned_t &D);
		void setPositionTargetGlobalINT(mavlink_set_position_target_global_int_t &D);
		void statusText(mavlink_statustext_t &D);
		void visionPositionEstimate(mavlink_vision_position_estimate_t &D);
		void visionSpeedEstimate(mavlink_vision_speed_estimate_t &D);

		// Cmd long
		void clComponentArmDisarm(bool bArm);
		void clDoFlightTermination(bool bTerminate);
		void clDoSetMode(int mode);
		void clDoSetServo(int iServo, int PWM);
		void clDoSetRelay(int iRelay, bool bRelay);
		void clDoSetHome(bool bUseCurrent, float r, float p, float y, float lat, float lon, float alt);
		void clGetHomePosition(void);
		void clNavSetYawSpeed(float yaw, float speed, float yawMode);
		void clNavTakeoff(float alt);
		void clNavRTL(void);
		void clSetMessageInterval(float id, float interval, float responseTarget);

		// Msg routing
		void setCmdRoute(uint32_t iCmd, bool bON);

		// Utility
		void sendSetMsgInterval(void);
		bool setMsgInterval(int id, uint64_t tInt);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Mavlink *)This)->update();
			return NULL;
		}

	public:
		mavlink_status_t m_status;
		vector<MAVLINK_PEER> m_vPeer;

		MavAttitude m_attitude;
		MavAttitudeQuaternion m_attitudeQuaternion;
		MavBatteryStatus m_batteryStatus;
		MavCommandAck m_commandAck;
		MavGlobalPositionINT m_globalPositionINT;
		MavGpsRawINT m_gpsRawINT;
		MavHeartbeat m_heartbeat;
		MavHighresIMU m_highresIMU;
		MavHomePosition m_homePosition;
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
		MavStatusText m_statusText;
		MavSysStatus m_sysStatus;
		MavScaledIMU m_scaledIMU;

		vector<MavMsgBase *> m_vpMsg;

	private:
		_IObase *m_pIO;
		int m_mySystemID;
		int m_myComponentID;
		int m_myType;
		int m_devSystemID;
		int m_devComponentID;
		int m_devType;

		uint8_t m_rBuf[MAV_N_BUF];
		int m_nRead;
		int m_iRead;
	};

}
#endif
