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

struct Time_Stamps
{
	uint64_t m_attitude;
	uint64_t m_battery_status;
	uint64_t m_global_position_int;
	uint64_t m_heartbeat;
	uint64_t m_highres_imu;
	uint64_t m_home_position;
	uint64_t m_local_position_ned;
	uint64_t m_mission_current;
	uint64_t m_mount_status;
	uint64_t m_param_set;
	uint64_t m_position_target_local_ned;
	uint64_t m_position_target_global_int;
	uint64_t m_radio_status;
	uint64_t m_raw_imu;
	uint64_t m_rc_channels;
	uint64_t m_rc_channels_override;
	uint64_t m_rc_channels_raw;
	uint64_t m_rc_channels_scaled;
	uint64_t m_sys_status;
	uint64_t m_scaled_imu;

	void init()
	{
		m_attitude = 0;
		m_battery_status = 0;
		m_global_position_int = 0;
		m_heartbeat = 0;
		m_highres_imu = 0;
		m_home_position = 0;
		m_local_position_ned = 0;
		m_mission_current = 0;
		m_mount_status = 0;
		m_param_set = 0;
		m_position_target_local_ned = 0;
		m_position_target_global_int = 0;
		m_radio_status = 0;
		m_raw_imu = 0;
		m_rc_channels = 0;
		m_rc_channels_override = 0;
		m_rc_channels_raw = 0;
		m_rc_channels_scaled = 0;
		m_sys_status = 0;
		m_scaled_imu = 0;
	}
};

// Struct containing information on the MAV we are currently connected to
struct Mavlink_Messages
{

	mavlink_attitude_t						m_attitude;
	mavlink_battery_status_t				m_battery_status;
	mavlink_command_ack_t					m_command_ack;
	mavlink_global_position_int_t			m_global_position_int;
	mavlink_heartbeat_t						m_heartbeat;
	mavlink_highres_imu_t					m_highres_imu;
	mavlink_home_position_t					m_home_position;
	mavlink_local_position_ned_t			m_local_position_ned;
	mavlink_mission_current_t				m_mission_current;
	mavlink_mount_status_t					m_mount_status;
	mavlink_param_set_t						m_param_set;
	mavlink_position_target_local_ned_t		m_position_target_local_ned;
	mavlink_position_target_global_int_t	m_position_target_global_int;
	mavlink_radio_status_t					m_radio_status;
	mavlink_raw_imu_t						m_raw_imu;
	mavlink_rc_channels_t					m_rc_channels;
	mavlink_rc_channels_override_t			m_rc_channels_override;
	mavlink_rc_channels_raw_t				m_rc_channels_raw;
	mavlink_rc_channels_scaled_t			m_rc_channels_scaled;
	mavlink_sys_status_t					m_sys_status;
	mavlink_scaled_imu_t					m_scaled_imu;

	Time_Stamps m_tStamps;

	void init(void)
	{
		m_attitude.yaw = 0;
		m_attitude.yawspeed = 0;
		m_attitude.pitch = 0;
		m_attitude.pitchspeed = 0;
		m_attitude.roll = 0;
		m_attitude.rollspeed = 0;
		m_global_position_int.alt = 0;
		m_global_position_int.lat = 0.0;
		m_global_position_int.lon = 0.0;
		m_global_position_int.relative_alt = 0;
		m_global_position_int.hdg = UINT16_MAX;

		m_heartbeat.custom_mode = 0;
		m_heartbeat.base_mode = 0;

		m_mission_current.seq = 0;

		m_local_position_ned.vx = 0;
		m_local_position_ned.vy = 0;
		m_local_position_ned.vz = 0;
		m_local_position_ned.x = 0;
		m_local_position_ned.y = 0;
		m_local_position_ned.z = 0;

		m_rc_channels_scaled.chan1_scaled = UINT16_MAX;
		m_rc_channels_scaled.chan2_scaled = UINT16_MAX;
		m_rc_channels_scaled.chan3_scaled = UINT16_MAX;
		m_rc_channels_scaled.chan4_scaled = UINT16_MAX;
		m_rc_channels_scaled.chan5_scaled = UINT16_MAX;
		m_rc_channels_scaled.chan6_scaled = UINT16_MAX;
		m_rc_channels_scaled.chan7_scaled = UINT16_MAX;
		m_rc_channels_scaled.chan8_scaled = UINT16_MAX;

		m_rc_channels_raw.chan1_raw = UINT16_MAX;
		m_rc_channels_raw.chan2_raw = UINT16_MAX;
		m_rc_channels_raw.chan3_raw = UINT16_MAX;
		m_rc_channels_raw.chan4_raw = UINT16_MAX;
		m_rc_channels_raw.chan5_raw = UINT16_MAX;
		m_rc_channels_raw.chan6_raw = UINT16_MAX;
		m_rc_channels_raw.chan7_raw = UINT16_MAX;
		m_rc_channels_raw.chan8_raw = UINT16_MAX;

		m_rc_channels.chancount = 0;
		m_rc_channels.chan1_raw = UINT16_MAX;
		m_rc_channels.chan2_raw = UINT16_MAX;
		m_rc_channels.chan3_raw = UINT16_MAX;
		m_rc_channels.chan4_raw = UINT16_MAX;
		m_rc_channels.chan5_raw = UINT16_MAX;
		m_rc_channels.chan6_raw = UINT16_MAX;
		m_rc_channels.chan7_raw = UINT16_MAX;
		m_rc_channels.chan8_raw = UINT16_MAX;
		m_rc_channels.chan9_raw = UINT16_MAX;
		m_rc_channels.chan10_raw = UINT16_MAX;
		m_rc_channels.chan11_raw = UINT16_MAX;
		m_rc_channels.chan12_raw = UINT16_MAX;
		m_rc_channels.chan13_raw = UINT16_MAX;
		m_rc_channels.chan14_raw = UINT16_MAX;
		m_rc_channels.chan15_raw = UINT16_MAX;
		m_rc_channels.chan16_raw = UINT16_MAX;
		m_rc_channels.chan17_raw = UINT16_MAX;
		m_rc_channels.chan18_raw = UINT16_MAX;
		m_rc_channels.rssi = 255;

		m_tStamps.init();
	}
};

struct MAVLINK_PEER
{
	void* m_pPeer;
	uint64_t m_pCmdRoute[MAV_N_CMD_U64]; //index of bit corresponds to Mavlink CMD ID

	void init(void)
	{
		m_pPeer = NULL;
		reset();
	}

	void reset(void)
	{
		for(int i=0; i<MAV_N_CMD_U64; i++)
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

		if(bON)
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

	bool init(void* pKiss);
	bool start(void);
	void draw(void);

	//Receive
	void handleMessages();
	bool readMessage(mavlink_message_t &message);

	//Send
	void writeMessage(mavlink_message_t message);

	//Cmd
	void cmdInt(mavlink_command_int_t& D);
	void distanceSensor(mavlink_distance_sensor_t& D);
	void globalVisionPositionEstimate(mavlink_global_vision_position_estimate_t& D);
	void gpsInput(mavlink_gps_input_t& D);
	void globalPositionInt(mavlink_global_position_int_t& D);
	void landingTarget(mavlink_landing_target_t& D);
	void mountConfigure(mavlink_mount_configure_t& D);
	void mountControl(mavlink_mount_control_t& D);
	void mountStatus(mavlink_mount_status_t& D);
	void param_set(mavlink_param_set_t& D);
	void positionTargetLocalNed(mavlink_position_target_local_ned_t& D);
	void positionTargetGlobalInt(mavlink_position_target_global_int_t& D);
	void rcChannelsOverride(mavlink_rc_channels_override_t& D);
	void requestDataStream(uint8_t stream_id, int rate);
	void heartbeat(void);
	void setAttitudeTarget(mavlink_set_attitude_target_t& D);
	void setMode(mavlink_set_mode_t& D);
	void setPositionTargetLocalNED(mavlink_set_position_target_local_ned_t& D);
	void setPositionTargetGlobalINT(mavlink_set_position_target_global_int_t& D);
	void visionPositionEstimate(mavlink_vision_position_estimate_t& D);

	//Cmd long
	void clComponentArmDisarm(bool bArm);
	void clDoSetMode(int mode);
	void clNavSetYawSpeed(float yaw, float speed, float yawMode);
	void clDoSetServo(int iServo, int PWM);
	void clDoSetRelay(int iRelay, bool bRelay);
	void clGetHomePosition(void);
	void clNavTakeoff(float alt);

	//Msg routing
	void setCmdRoute(uint32_t iCmd, bool bON);

	//Utility
	int16_t* getRCinScaled(int iChan);
	uint16_t* getRCinRaw(int iChan);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Mavlink *) This)->update();
		return NULL;
	}

public:
	Mavlink_Messages m_mavMsg;
	mavlink_status_t m_status;
	vector<MAVLINK_PEER> m_vPeer;

private:
	_IOBase* m_pIO;
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

/*
I believe this is the one
The board itself is 1m.  The outter edge of the red circle is 93cm diameter
each red ring is 7cm wide
the central red circle is 9cm diameter

MAVLink command sequence..
1. OpenKai should send a heartbeat at 1hz.  http://mavlink.org/messages/common#HEARTBEAT
Inside the HEARTBEAT message (and any other message), openkai needs to specify it's sys-id and component-id.
ideally the sys-id should match the vehicle's sysid.
By default the vehicle's sys-id is "1" so you could just hardcode that for now.
Eventually openKai should instead listen for heartbeat messages on it's serial port and should look for the first heartbeat messages with a
type != MAV_TYPE_GCS ("6") and grab that system id and use it.

the component id can be anything except 0.
there's a list of suggested component-ids on this page (search for MAV_COMPONENT) http://mavlink.org/messages/common.  but it doesn't really matter.  Maybe we could use MAV_COMP_ID_PATHPLANNER ("195")

The Heartbeat's "type" should be MAV_TYPE_ONBOARD_CONTROLLER (18)
"base_mode" can be anything.
maybe we will eventually assign values for when OpenKai is providing precision-landing updates vs follow-me updates.. but for now just make it "0" I think
"autopilot" should be "0"
"custom_mode" can be "0"
"system_status" should be MAV_STATE_ACTIVE ("4")

Next, when openkai sees the target is should send LANDING_TARGET messages maybe as quickly as it can.
10hz would be nice.  over 50hz is too fast.
http://mavlink.org/messages/common#LANDING_TARGET
	"time_usec" is just the TX1's system time in sec * 1,000,000.  We don't use this anyway
	"target_num" = "0" (for now anyway)
	"frame" = MAV_FRAME_BODY_NED ("8")
	"angle_x" : the horizontal angle to the target in radians.
		0 = the target is right in the middle of the screen.
		negative means the target is to the left of center,
		positive means it's to the right of center.
		OpenKai will need to know the field-of-view of the camera.
		So for example, I measured the field of view of the Logitech 920c and horizontall it's 70.42degrees).
		So if the center of the target was on the left most edge of the image we would send "angle_x" as -0.61 radians (= -35degrees)
	"angle_y" : the vertical angle to the target in radians.
		0 = middle.  to be honest, I'm not exactly sure if negative is up or down.
	"distance" - we don't use this now.
		It can be estimated even using non-stereo cameras if you know the actual size of the object.
		As you probably know, the apparent diameter of the object halves if it's distance is doubled.
		Anyway, we don't use it anyway for now
	"size_x", "size_y" - the horizontal and vertical size of the object in radians.  Also not used.

OpenKAI -> Autopilot
MAVLINK_MSG_ID_HEARTBEAT 0
MAVLINK_MSG_ID_COMMAND_INT 75
MAVLINK_MSG_ID_DISTANCE_SENSOR 132
MAVLINK_MSG_ID_GLOBAL_VISION_POSITION_ESTIMATE 101
MAVLINK_MSG_ID_GPS_INPUT 232
MAVLINK_MSG_ID_GLOBAL_POSITION_INT 33
MAVLINK_MSG_ID_LANDING_TARGET 149
MAVLINK_MSG_ID_MOUNT_CONFIGURE 156
MAVLINK_MSG_ID_MOUNT_CONTROL 157
MAVLINK_MSG_ID_MOUNT_STATUS 158
MAVLINK_MSG_ID_PARAM_SET 23
MAVLINK_MSG_ID_SET_POSITION_TARGET_LOCAL_NED 84
MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED 85
MAVLINK_MSG_ID_SET_POSITION_TARGET_GLOBAL_INT 86
MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT 87
MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE 70
MAVLINK_MSG_ID_REQUEST_DATA_STREAM 66
MAVLINK_MSG_ID_SET_ATTITUDE_TARGET 82
MAVLINK_MSG_ID_SET_MODE 11
MAVLINK_MSG_ID_VISION_POSITION_ESTIMATE 102
MAVLINK_MSG_ID_COMMAND_LONG 76
MAV_CMD_COMPONENT_ARM_DISARM
MAV_CMD_DO_SET_MODE
MAV_CMD_NAV_SET_YAW_SPEED
MAV_CMD_DO_SET_SERVO
MAV_CMD_GET_HOME_POSITION

Autopilot -> OpenKAI
MAVLINK_MSG_ID_ATTITUDE
MAVLINK_MSG_ID_BATTERY_STATUS
MAVLINK_MSG_ID_COMMAND_ACK
MAVLINK_MSG_ID_GLOBAL_POSITION_INT
MAVLINK_MSG_ID_HEARTBEAT
MAVLINK_MSG_ID_HIGHRES_IMU
MAVLINK_MSG_ID_HOME_POSITION
MAVLINK_MSG_ID_LOCAL_POSITION_NED
MAVLINK_MSG_ID_MISSION_CURRENT
MAVLINK_MSG_ID_MOUNT_STATUS
MAVLINK_MSG_ID_PARAM_SET
MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED
MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT
MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE
MAVLINK_MSG_ID_RC_CHANNELS_RAW
MAVLINK_MSG_ID_RADIO_STATUS
MAVLINK_MSG_ID_RAW_IMU
MAVLINK_MSG_ID_SCALED_IMU
MAVLINK_MSG_ID_SYS_STATUS

 */
