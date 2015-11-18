#pragma once

#include "../Base/common.h"
#include "../include/mavlink/common/mavlink.h"
#include "../IO/SerialPort.h"

#include <signal.h>
#include <time.h>
#include <sys/time.h>

// ------------------------------------------------------------------------------
//   Defines
// ------------------------------------------------------------------------------

/**
 * Defines for mavlink_set_position_target_local_ned_t.type_mask
 *
 * Bitmask to indicate which dimensions should be ignored by the vehicle
 *
 * a value of 0b0000000000000000 or 0b0000001000000000 indicates that none of
 * the setpoint dimensions should be ignored.
 *
 * If bit 10 is set the floats afx afy afz should be interpreted as force
 * instead of acceleration.
 *
 * Mapping:
 * bit 1: x,
 * bit 2: y,
 * bit 3: z,
 * bit 4: vx,
 * bit 5: vy,
 * bit 6: vz,
 * bit 7: ax,
 * bit 8: ay,
 * bit 9: az,
 * bit 10: is force setpoint,
 * bit 11: yaw,
 * bit 12: yaw rate
 * remaining bits unused
 *
 * Combine bitmasks with bitwise &
 *
 * Example for position and yaw angle:
 * uint16_t type_mask =
 *     MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_POSITION &
 *     MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_YAW_ANGLE;
 */

// bit number  876543210987654321
#define MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_POSITION     0b0000110111111000
#define MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_VELOCITY     0b0000110111000111
#define MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_ACCELERATION 0b0000110000111111
#define MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_FORCE        0b0000111000111111
#define MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_YAW_ANGLE    0b0000100111111111
#define MAVLINK_MSG_SET_POSITION_TARGET_LOCAL_NED_YAW_RATE     0b0000010111111111

// ------------------------------------------------------------------------------
//   Data Structures
// ------------------------------------------------------------------------------

struct Time_Stamps
{
	Time_Stamps()
	{
		reset_timestamps();
	}

	uint64_t heartbeat;
	uint64_t sys_status;
	uint64_t battery_status;
	uint64_t radio_status;
	uint64_t local_position_ned;
	uint64_t global_position_int;
	uint64_t position_target_local_ned;
	uint64_t position_target_global_int;
	uint64_t highres_imu;
	uint64_t attitude;

	void reset_timestamps()
	{
		heartbeat = 0;
		sys_status = 0;
		battery_status = 0;
		radio_status = 0;
		local_position_ned = 0;
		global_position_int = 0;
		position_target_local_ned = 0;
		position_target_global_int = 0;
		highres_imu = 0;
		attitude = 0;
	}

};

// Struct containing information on the MAV we are currently connected to

struct Mavlink_Messages
{

	int sysid;
	int compid;

	mavlink_heartbeat_t heartbeat;
	mavlink_sys_status_t sys_status;
	mavlink_battery_status_t battery_status;
	mavlink_radio_status_t radio_status;
	mavlink_local_position_ned_t local_position_ned;
	mavlink_global_position_int_t global_position_int;
	mavlink_position_target_local_ned_t position_target_local_ned;
	mavlink_position_target_global_int_t position_target_global_int;
	mavlink_highres_imu_t highres_imu;
	mavlink_attitude_t attitude;
	Time_Stamps time_stamps;

	void reset_timestamps()
	{
		time_stamps.reset_timestamps();
	}

};

#define TRD_INTERVAL_MI_USEC 10
#define NUM_MSG_HANDLE 10

namespace kai
{

class MavlinkInterface: public ThreadBase
{
public:
	MavlinkInterface();
	~MavlinkInterface();

	void setSerial(string name, int baudrate);
	bool open(void);
	void close(void);

	bool start(void);
	bool complete(void);
	void stop(void);
	void waitForComplete(void);

	//Receive
	void handleMessages();
	bool readMessage(mavlink_message_t &message);

	//Send
	int  writeMessage(mavlink_message_t message);
	void requestDataStream(uint8_t stream_id, int rate);

	//Commands
	int  toggleOffboardControl(bool bEnable);


	void update_setpoint(mavlink_set_position_target_local_ned_t setpoint);
	void write_setpoint();

	void set_position(float x, float y, float z,
			mavlink_set_position_target_local_ned_t &sp);
	void set_velocity(float vx, float vy, float vz,
			mavlink_set_position_target_local_ned_t &sp);
	void set_acceleration(float ax, float ay, float az,
			mavlink_set_position_target_local_ned_t &sp);
	void set_yaw(float yaw, mavlink_set_position_target_local_ned_t &sp);
	void set_yaw_rate(float yaw_rate,
			mavlink_set_position_target_local_ned_t &sp);

public:
	bool m_bSerialConnected;

	string m_sportName;
	SerialPort* m_pSerialPort;
	int m_baudRate;

	int system_id;
	int autopilot_id;
	int companion_id;
	bool m_bControlling;

	Mavlink_Messages current_messages;
	mavlink_set_position_target_local_ned_t initial_position;
	mavlink_status_t last_status;



	//Read Thread
	pthread_t m_threadID;
	bool m_bThreadON;

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((MavlinkInterface *) This)->update();
		return NULL;
	}

};

}

