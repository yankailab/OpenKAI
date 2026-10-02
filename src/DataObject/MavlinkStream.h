/*
 * MavlinkStream.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__MavlinkStream__H_
#define OpenKAI_src__DataStream__MavlinkStream__H_

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
#include "../Dependencies/c_library_v2/ardupilotmega/mavlink.h"
#include "../Dependencies/c_library_v2/mavlink_conversions.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include "DataObjBase.h"

namespace kai
{
	typedef void (*CbMavMsg)(void *pMsg, void *pInst);
	struct MavCallback
	{
		CbMavMsg m_pCbRecv = NULL;
		void *m_pCbInst = NULL;

		void callback(void *pMavMsg)
		{
			NULL_(m_pCbRecv);
			m_pCbRecv(pMavMsg, m_pCbInst);
		}
	};

	class MavMsgBase
	{
	public:
		MavMsgBase() {
		};

		virtual ~MavMsgBase(void) {};

		uint32_t getID(void)
		{
			return m_id;
		}

		virtual const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			return m_msgT;
		}

		virtual void decode(const mavlink_message_t &msg)
		{
		}

		void setDesiredInterval(uint64_t tIntervalNsec)
		{
			m_tDesiredInterval = tIntervalNsec;
		}

		uint64_t getDesiredInterval(void)
		{
			return m_tDesiredInterval;
		}

		bool bOnTime(void)
		{
			IF__(m_tActualInterval < m_tDesiredInterval + m_tIntervalDelayAllowed, true);

			return false;
		}

		bool bValid(void)
		{
			return m_tStamp > 0;
		}

		void updateTstamp(uint64_t tStamp = 0)
		{
			if (tStamp == 0)
				tStamp = getTns();

			m_tActualInterval = tStamp - m_tStamp;
			m_tStamp = tStamp;
		}

		uint64_t getTstamp(void)
		{
			return m_tStamp;
		}

		void callbackAll(void)
		{
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
				IF__((c.m_pCbRecv == pCb) && (c.m_pCbInst == pInst), true);
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

			for (auto it = m_vCbRecv.begin(); it != m_vCbRecv.end(); ++it)
			{
				MavCallback *pC = &(*it);
				IF_CONT((pC->m_pCbRecv != pCb) || (pC->m_pCbInst != pInst));

				m_vCbRecv.erase(it);

				return;
			}
		}

		void clearAllCbRecv(void)
		{
			m_vCbRecv.clear();
		}

	protected:
		uint32_t m_id = 0x7fffffff;
		uint64_t m_tStamp = 0;

		int64_t m_tDesiredInterval = -1;
		int64_t m_tActualInterval = LONG_MAX;
		int64_t m_tIntervalDelayAllowed = NSEC_SEC / 10;

		vector<MavCallback> m_vCbRecv;

		mavlink_message_t m_msgT;
	};

	class MavAttitude : public MavMsgBase
	{
	public:
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

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_attitude_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_attitude_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_boot_ms = tStamp;
			if (tStamp == 0)
				m_msg.time_boot_ms = getTbootMs();

			mavlink_msg_attitude_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_attitude_t m_msg;
	};

	class MavAttitudeQuaternion : public MavMsgBase
	{
	public:
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

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_attitude_quaternion_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_attitude_quaternion_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_boot_ms = tStamp;
			if (tStamp == 0)
				m_msg.time_boot_ms = getTbootMs();

			mavlink_msg_attitude_quaternion_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_attitude_quaternion_t m_msg;
	};

	class MavBatteryStatus : public MavMsgBase
	{
	public:
		MavBatteryStatus()
		{
			m_id = MAVLINK_MSG_ID_BATTERY_STATUS;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_battery_status_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_battery_status_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			mavlink_msg_battery_status_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_battery_status_t m_msg;
	};

	class MavCommandAck : public MavMsgBase
	{
	public:
		MavCommandAck()
		{
			m_id = MAVLINK_MSG_ID_COMMAND_ACK;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_command_ack_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_command_ack_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_command_ack_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_command_ack_t m_msg;
	};

	class MavCommandInt : public MavMsgBase
	{
	public:
		MavCommandInt()
		{
			m_id = MAVLINK_MSG_ID_COMMAND_INT;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_command_int_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_command_int_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_command_int_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_command_int_t m_msg;
	};

	class MavCommandLong : public MavMsgBase
	{
	public:
		MavCommandLong()
		{
			m_id = MAVLINK_MSG_ID_COMMAND_LONG;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_command_long_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_command_long_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_command_long_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_command_long_t m_msg;
	};

	class MavDistanceSensor : public MavMsgBase
	{
	public:
		MavDistanceSensor()
		{
			m_id = MAVLINK_MSG_ID_DISTANCE_SENSOR;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_distance_sensor_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_distance_sensor_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_boot_ms = tStamp;
			if (tStamp == 0)
				m_msg.time_boot_ms = getTbootMs();

			mavlink_msg_distance_sensor_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_distance_sensor_t m_msg;
	};

	class MavGlobalPositionINT : public MavMsgBase
	{
	public:
		MavGlobalPositionINT()
		{
			m_id = MAVLINK_MSG_ID_GLOBAL_POSITION_INT;

			m_msg.alt = 0;
			m_msg.lat = 0.0;
			m_msg.lon = 0.0;
			m_msg.relative_alt = 0;
			m_msg.hdg = UINT16_MAX;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_global_position_int_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_global_position_int_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_boot_ms = tStamp;
			if (tStamp == 0)
				m_msg.time_boot_ms = getTbootMs();

			mavlink_msg_global_position_int_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_global_position_int_t m_msg;
	};

	class MavGlobalVisionPositionEstimate : public MavMsgBase
	{
	public:
		MavGlobalVisionPositionEstimate()
		{
			m_id = MAVLINK_MSG_ID_GLOBAL_VISION_POSITION_ESTIMATE;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_global_vision_position_estimate_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_global_vision_position_estimate_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.usec = tStamp;
			if (tStamp == 0)
				m_msg.usec = getTbootMs() * USEC_MSEC;

			mavlink_msg_global_vision_position_estimate_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_global_vision_position_estimate_t m_msg;
	};

	class MavGpsInput : public MavMsgBase
	{
	public:
		MavGpsInput()
		{
			m_id = MAVLINK_MSG_ID_GPS_INPUT;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_gps_input_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_gps_input_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_usec = tStamp;
			if (tStamp == 0)
				m_msg.time_usec = getTbootMs() * USEC_MSEC;

			mavlink_msg_gps_input_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_gps_input_t m_msg;
	};

	class MavGpsRawINT : public MavMsgBase
	{
	public:
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

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_gps_raw_int_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_gps_raw_int_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_usec = tStamp;
			if (tStamp == 0)
				m_msg.time_usec = getTbootMs() * USEC_MSEC;

			mavlink_msg_gps_raw_int_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_gps_raw_int_t m_msg;
	};

	class MavGpsRTCMdata : public MavMsgBase
	{
	public:
		MavGpsRTCMdata()
		{
			m_id = MAVLINK_MSG_ID_GPS_RTCM_DATA;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_gps_rtcm_data_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_gps_rtcm_data_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			mavlink_msg_gps_rtcm_data_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_gps_rtcm_data_t m_msg;
	};

	class MavHeartbeat : public MavMsgBase
	{
	public:
		MavHeartbeat()
		{
			m_id = MAVLINK_MSG_ID_HEARTBEAT;

			m_msg.custom_mode = 0;
			m_msg.base_mode = 0;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_heartbeat_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_heartbeat_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			mavlink_msg_heartbeat_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_heartbeat_t m_msg;
	};

	class MavHighresIMU : public MavMsgBase
	{
	public:
		MavHighresIMU()
		{
			m_id = MAVLINK_MSG_ID_HIGHRES_IMU;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_highres_imu_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_highres_imu_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_usec = tStamp;
			if (tStamp == 0)
				m_msg.time_usec = getTbootMs() * USEC_MSEC;

			mavlink_msg_highres_imu_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_highres_imu_t m_msg;
	};

	class MavHomePosition : public MavMsgBase
	{
	public:
		MavHomePosition()
		{
			m_id = MAVLINK_MSG_ID_HOME_POSITION;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_home_position_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_home_position_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_usec = tStamp;
			if (tStamp == 0)
				m_msg.time_usec = getTbootMs() * USEC_MSEC;

			mavlink_msg_home_position_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_home_position_t m_msg;
	};

	class MavLandingTarget : public MavMsgBase
	{
	public:
		MavLandingTarget()
		{
			m_id = MAVLINK_MSG_ID_LANDING_TARGET;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_landing_target_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_landing_target_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_usec = tStamp;
			if (tStamp == 0)
				m_msg.time_usec = getTbootMs() * USEC_MSEC;

			mavlink_msg_landing_target_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_landing_target_t m_msg;
	};

	class MavLocalPositionNED : public MavMsgBase
	{
	public:
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

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_local_position_ned_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_local_position_ned_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_boot_ms = tStamp;
			if (tStamp == 0)
				m_msg.time_boot_ms = getTbootMs();

			mavlink_msg_local_position_ned_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_local_position_ned_t m_msg;
	};

	class MavMissionCount : public MavMsgBase
	{
	public:
		MavMissionCount()
		{
			m_id = MAVLINK_MSG_ID_MISSION_COUNT;

			m_msg.count = 0;
			m_msg.mission_type = 0;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_mission_count_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_mission_count_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_mission_count_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_mission_count_t m_msg;
	};

	class MavMissionCurrent : public MavMsgBase
	{
	public:
		MavMissionCurrent()
		{
			m_id = MAVLINK_MSG_ID_MISSION_CURRENT;

			m_msg.seq = 0;
			m_msg.total = 0;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_mission_current_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_mission_current_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			mavlink_msg_mission_current_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_mission_current_t m_msg;
	};

	class MavMissionRequestList : public MavMsgBase
	{
	public:
		MavMissionRequestList()
		{
			m_id = MAVLINK_MSG_ID_MISSION_REQUEST_LIST;

			m_msg.mission_type = 0;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_mission_request_list_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_mission_request_list_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_mission_request_list_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_mission_request_list_t m_msg;
	};

	class MavMissionRequestInt : public MavMsgBase
	{
	public:
		MavMissionRequestInt()
		{
			m_id = MAVLINK_MSG_ID_MISSION_REQUEST_INT;

			m_msg.mission_type = 0;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_mission_request_int_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_mission_request_int_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_mission_request_int_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_mission_request_int_t m_msg;
	};

	class MavMissionItemInt : public MavMsgBase
	{
	public:
		MavMissionItemInt()
		{
			m_id = MAVLINK_MSG_ID_MISSION_ITEM_INT;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_mission_item_int_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_mission_item_int_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_mission_item_int_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_mission_item_int_t m_msg;
	};

	class MavMissionAck : public MavMsgBase
	{
	public:
		MavMissionAck()
		{
			m_id = MAVLINK_MSG_ID_MISSION_ACK;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_mission_ack_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_mission_ack_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_mission_ack_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_mission_ack_t m_msg;
	};

	class MavMissionSetCurrent : public MavMsgBase
	{
	public:
		MavMissionSetCurrent()
		{
			m_id = MAVLINK_MSG_ID_MISSION_SET_CURRENT;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_mission_set_current_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_mission_set_current_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_mission_set_current_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_mission_set_current_t m_msg;
	};

	class MavMissionClearAll : public MavMsgBase
	{
	public:
		MavMissionClearAll()
		{
			m_id = MAVLINK_MSG_ID_MISSION_CLEAR_ALL;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_mission_clear_all_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_mission_clear_all_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_mission_clear_all_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_mission_clear_all_t m_msg;
	};

	class MavMissionItemReached : public MavMsgBase
	{
	public:
		MavMissionItemReached()
		{
			m_id = MAVLINK_MSG_ID_MISSION_ITEM_REACHED;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_mission_item_reached_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_mission_item_reached_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			mavlink_msg_mission_item_reached_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_mission_item_reached_t m_msg;
	};

	class MavMountConfigure : public MavMsgBase
	{
	public:
		MavMountConfigure()
		{
			m_id = MAVLINK_MSG_ID_MOUNT_CONFIGURE;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_mount_configure_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_mount_configure_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_mount_configure_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_mount_configure_t m_msg;
	};

	class MavMountControl : public MavMsgBase
	{
	public:
		MavMountControl()
		{
			m_id = MAVLINK_MSG_ID_MOUNT_CONTROL;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_mount_control_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_mount_control_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_mount_control_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_mount_control_t m_msg;
	};

	class MavMountStatus : public MavMsgBase
	{
	public:
		MavMountStatus()
		{
			m_id = MAVLINK_MSG_ID_MOUNT_STATUS;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_mount_status_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_mount_status_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_mount_status_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_mount_status_t m_msg;
	};

	class MavParamRequestRead : public MavMsgBase
	{
	public:
		MavParamRequestRead()
		{
			m_id = MAVLINK_MSG_ID_PARAM_REQUEST_READ;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_param_request_read_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_param_request_read_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_param_request_read_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_param_request_read_t m_msg;
	};

	class MavParamSet : public MavMsgBase
	{
	public:
		MavParamSet()
		{
			m_id = MAVLINK_MSG_ID_PARAM_SET;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_param_set_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_param_set_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_param_set_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_param_set_t m_msg;
	};

	class MavParamValue : public MavMsgBase
	{
	public:
		MavParamValue()
		{
			m_id = MAVLINK_MSG_ID_PARAM_VALUE;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_param_value_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_param_value_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			mavlink_msg_param_value_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_param_value_t m_msg;
	};

	class MavPositionTargetLocalNED : public MavMsgBase
	{
	public:
		MavPositionTargetLocalNED()
		{
			m_id = MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_position_target_local_ned_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_position_target_local_ned_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_boot_ms = tStamp;
			if (tStamp == 0)
				m_msg.time_boot_ms = getTbootMs();

			mavlink_msg_position_target_local_ned_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_position_target_local_ned_t m_msg;
	};

	class MavPositionTargetGlobalINT : public MavMsgBase
	{
	public:
		MavPositionTargetGlobalINT()
		{
			m_id = MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_position_target_global_int_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_position_target_global_int_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_boot_ms = tStamp;
			if (tStamp == 0)
				m_msg.time_boot_ms = getTbootMs();

			mavlink_msg_position_target_global_int_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_position_target_global_int_t m_msg;
	};

	class MavRadioStatus : public MavMsgBase
	{
	public:
		MavRadioStatus()
		{
			m_id = MAVLINK_MSG_ID_RADIO_STATUS;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_radio_status_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_radio_status_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			mavlink_msg_radio_status_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_radio_status_t m_msg;
	};

	class MavRawIMU : public MavMsgBase
	{
	public:
		MavRawIMU()
		{
			m_id = MAVLINK_MSG_ID_RAW_IMU;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_raw_imu_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_raw_imu_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_usec = tStamp;
			if (tStamp == 0)
				m_msg.time_usec = getTbootMs() * USEC_MSEC;

			mavlink_msg_raw_imu_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_raw_imu_t m_msg;
	};

	class MavRcChannels : public MavMsgBase
	{
	public:
		uint16_t *m_pChan[19] = {
			NULL, &m_msg.chan1_raw, &m_msg.chan2_raw, &m_msg.chan3_raw,
			&m_msg.chan4_raw, &m_msg.chan5_raw, &m_msg.chan6_raw, &m_msg.chan7_raw,
			&m_msg.chan8_raw, &m_msg.chan9_raw, &m_msg.chan10_raw, &m_msg.chan11_raw,
			&m_msg.chan12_raw, &m_msg.chan13_raw, &m_msg.chan14_raw, &m_msg.chan15_raw,
			&m_msg.chan16_raw, &m_msg.chan17_raw, &m_msg.chan18_raw};

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
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_rc_channels_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_rc_channels_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_boot_ms = tStamp;
			if (tStamp == 0)
				m_msg.time_boot_ms = getTbootMs();

			mavlink_msg_rc_channels_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

		uint16_t getRC(int iChan)
		{
			if (iChan <= 0 || iChan > 18)
				return UINT16_MAX;

			return *m_pChan[iChan];
		}

	protected:
		mavlink_rc_channels_t m_msg;
	};

	class MavRcChannelsOverride : public MavMsgBase
	{
	public:
		MavRcChannelsOverride()
		{
			m_id = MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_rc_channels_override_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_rc_channels_override_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_rc_channels_override_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_rc_channels_override_t m_msg;
	};

	class MavRequestDataStream : public MavMsgBase
	{
	public:
		MavRequestDataStream()
		{
			m_id = MAVLINK_MSG_ID_REQUEST_DATA_STREAM;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_request_data_stream_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_request_data_stream_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_request_data_stream_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_request_data_stream_t m_msg;
	};

	class MavServoOutputRaw : public MavMsgBase
	{
	public:
		MavServoOutputRaw()
		{
			m_id = MAVLINK_MSG_ID_SERVO_OUTPUT_RAW;

			m_msg.port = 0;
			m_msg.servo1_raw = 0;
			m_msg.servo2_raw = 0;
			m_msg.servo3_raw = 0;
			m_msg.servo4_raw = 0;
			m_msg.servo5_raw = 0;
			m_msg.servo6_raw = 0;
			m_msg.servo7_raw = 0;
			m_msg.servo8_raw = 0;
			m_msg.servo9_raw = 0;
			m_msg.servo10_raw = 0;
			m_msg.servo11_raw = 0;
			m_msg.servo12_raw = 0;
			m_msg.servo13_raw = 0;
			m_msg.servo14_raw = 0;
			m_msg.servo15_raw = 0;
			m_msg.servo16_raw = 0;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_servo_output_raw_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_servo_output_raw_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_usec = tStamp;
			if (tStamp == 0)
				m_msg.time_usec = getTbootMs() * USEC_MSEC;

			mavlink_msg_servo_output_raw_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

		uint16_t getServo(int iServo)
		{
			switch (iServo)
			{
			case 1:
				return m_msg.servo1_raw;
			case 2:
				return m_msg.servo2_raw;
			case 3:
				return m_msg.servo3_raw;
			case 4:
				return m_msg.servo4_raw;
			case 5:
				return m_msg.servo5_raw;
			case 6:
				return m_msg.servo6_raw;
			case 7:
				return m_msg.servo7_raw;
			case 8:
				return m_msg.servo8_raw;
			case 9:
				return m_msg.servo9_raw;
			case 10:
				return m_msg.servo10_raw;
			case 11:
				return m_msg.servo11_raw;
			case 12:
				return m_msg.servo12_raw;
			case 13:
				return m_msg.servo13_raw;
			case 14:
				return m_msg.servo14_raw;
			case 15:
				return m_msg.servo15_raw;
			case 16:
				return m_msg.servo16_raw;
			default:
				return 0;
			}
		}

	protected:
		mavlink_servo_output_raw_t m_msg;
	};

	class MavSetAttitudeTarget : public MavMsgBase
	{
	public:
		MavSetAttitudeTarget()
		{
			m_id = MAVLINK_MSG_ID_SET_ATTITUDE_TARGET;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_set_attitude_target_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_set_attitude_target_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_boot_ms = tStamp;
			if (tStamp == 0)
				m_msg.time_boot_ms = getTbootMs();

			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_set_attitude_target_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_set_attitude_target_t m_msg;
	};

	class MavSetMode : public MavMsgBase
	{
	public:
		MavSetMode()
		{
			m_id = MAVLINK_MSG_ID_SET_MODE;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_set_mode_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_set_mode_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.target_system = sysIDto;

			mavlink_msg_set_mode_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_set_mode_t m_msg;
	};

	class MavSetPositionTargetLocalNED : public MavMsgBase
	{
	public:
		MavSetPositionTargetLocalNED()
		{
			m_id = MAVLINK_MSG_ID_SET_POSITION_TARGET_LOCAL_NED;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_set_position_target_local_ned_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_set_position_target_local_ned_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_boot_ms = tStamp;
			if (tStamp == 0)
				m_msg.time_boot_ms = getTbootMs();

			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_set_position_target_local_ned_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_set_position_target_local_ned_t m_msg;
	};

	class MavSetPositionTargetGlobalINT : public MavMsgBase
	{
	public:
		MavSetPositionTargetGlobalINT()
		{
			m_id = MAVLINK_MSG_ID_SET_POSITION_TARGET_GLOBAL_INT;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_set_position_target_global_int_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_set_position_target_global_int_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_boot_ms = tStamp;
			if (tStamp == 0)
				m_msg.time_boot_ms = getTbootMs();

			m_msg.target_system = sysIDto;
			m_msg.target_component = comIDto;

			mavlink_msg_set_position_target_global_int_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_set_position_target_global_int_t m_msg;
	};

	class MavStatusText : public MavMsgBase
	{
	public:
		MavStatusText()
		{
			m_id = MAVLINK_MSG_ID_STATUSTEXT;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_statustext_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_statustext_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			mavlink_msg_statustext_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_statustext_t m_msg;
	};

	class MavSysStatus : public MavMsgBase
	{
	public:
		MavSysStatus()
		{
			m_id = MAVLINK_MSG_ID_SYS_STATUS;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_sys_status_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_sys_status_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			mavlink_msg_sys_status_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_sys_status_t m_msg;
	};

	class MavScaledIMU : public MavMsgBase
	{
	public:
		MavScaledIMU()
		{
			m_id = MAVLINK_MSG_ID_SCALED_IMU;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_scaled_imu_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_scaled_imu_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.time_boot_ms = tStamp;
			if (tStamp == 0)
				m_msg.time_boot_ms = getTbootMs();

			mavlink_msg_scaled_imu_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_scaled_imu_t m_msg;
	};

	class MavVisionPositionEstimate : public MavMsgBase
	{
	public:
		MavVisionPositionEstimate()
		{
			m_id = MAVLINK_MSG_ID_VISION_POSITION_ESTIMATE;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_vision_position_estimate_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_vision_position_estimate_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.usec = tStamp;
			if (tStamp == 0)
				m_msg.usec = getTbootMs() * USEC_MSEC;

			mavlink_msg_vision_position_estimate_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_vision_position_estimate_t m_msg;
	};

	class MavVisionSpeedEstimate : public MavMsgBase
	{
	public:
		MavVisionSpeedEstimate()
		{
			m_id = MAVLINK_MSG_ID_VISION_SPEED_ESTIMATE;
		}

		void decode(const mavlink_message_t &msg)
		{
			mavlink_msg_vision_speed_estimate_decode(&msg, &m_msg);
			updateTstamp();
			callbackAll();
		}

		void set(const mavlink_vision_speed_estimate_t &msg)
		{
			m_msg = msg;
			updateTstamp();
		}

		const mavlink_message_t &encode(uint8_t sysIDfrom, uint8_t comIDfrom, uint8_t sysIDto, uint8_t comIDto, uint64_t tStamp = 0)
		{
			m_msg.usec = tStamp;
			if (tStamp == 0)
				m_msg.usec = getTbootMs() * USEC_MSEC;

			mavlink_msg_vision_speed_estimate_encode(sysIDfrom, comIDfrom, &m_msgT, &m_msg);

			return m_msgT;
		}

	protected:
		mavlink_vision_speed_estimate_t m_msg;
	};

	class MavlinkStream : public DataObjBase
	{
	public:
		MavlinkStream();
		virtual ~MavlinkStream();
		bool loadConfig(void);
		bool saveConfig(bool bExport = false);
		void console(void *pConsole) override;

		bool decode(const mavlink_message_t& msg);								// caller add a msg received from IO
		void getMsgQueue(vector<MavMsgBase*>& vMsg, uint64_t tStampFrom = 0);	// caller get the list of msgs to be written to IO
		void clearMsgQueue(size_t nMsg = 0);

		void sendSetMsgInterval(void);
		bool setMsgInterval(int id, uint64_t tInt);

		// caller call these functions to add a msg to send to FC
		void attitude(mavlink_attitude_t &D);
		void attitudeQuaternion(mavlink_attitude_quaternion_t &D);
		void batteryStatus(mavlink_battery_status_t &D);
		void commandAck(mavlink_command_ack_t &D);
		void cmdInt(mavlink_command_int_t &D);
		void cmdLong(mavlink_command_long_t &D);
		void distanceSensor(mavlink_distance_sensor_t &D);
		void globalPositionInt(mavlink_global_position_int_t &D);
		void globalVisionPositionEstimate(mavlink_global_vision_position_estimate_t &D);
		void gpsInput(mavlink_gps_input_t &D);
		void gpsRawINT(mavlink_gps_raw_int_t &D);
		void gpsRTCMdata(mavlink_gps_rtcm_data_t &D);
		void highresIMU(mavlink_highres_imu_t &D);
		void homePosition(mavlink_home_position_t &D);
		void landingTarget(mavlink_landing_target_t &D);
		void localPositionNED(mavlink_local_position_ned_t &D);

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
		void radioStatus(mavlink_radio_status_t &D);
		void rawIMU(mavlink_raw_imu_t &D);
		void rcChannels(mavlink_rc_channels_t &D);
		void rcChannelsOverride(mavlink_rc_channels_override_t &D);
		void requestDataStream(mavlink_request_data_stream_t &D);
		void requestDataStream(uint8_t stream_id, int rate);
		void heartbeat(mavlink_heartbeat_t &D);
		void servoOutputRaw(mavlink_servo_output_raw_t &D);
		void setAttitudeTarget(mavlink_set_attitude_target_t &D);
		void setMode(mavlink_set_mode_t &D);
		void setPositionTargetLocalNED(mavlink_set_position_target_local_ned_t &D);
		void setPositionTargetGlobalINT(mavlink_set_position_target_global_int_t &D);
		void statusText(mavlink_statustext_t &D);
		void sysStatus(mavlink_sys_status_t &D);
		void scaledIMU(mavlink_scaled_imu_t &D);
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

	protected:
		void addMsgQueue(MavMsgBase* pM);
		std::shared_mutex m_sMutex;
		vector<MavMsgBase *> m_vpMsgQueue;
		int m_nMsgQueue = 0;

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
	};

}
#endif
