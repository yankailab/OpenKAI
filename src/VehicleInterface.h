#pragma once

#include <stdio.h>
#include "include/mavlink/common/mavlink.h"
#include "SerialPort.h"
//#include <signal.h>
//#include <time.h>
//#include <sys/time.h>

// helper functions
/*
uint64_t get_time_usec();
void set_position(float x, float y, float z, mavlink_set_position_target_local_ned_t &sp);
void set_velocity(float vx, float vy, float vz, mavlink_set_position_target_local_ned_t &sp);
void set_acceleration(float ax, float ay, float az, mavlink_set_position_target_local_ned_t &sp);
void set_yaw(float yaw, mavlink_set_position_target_local_ned_t &sp);
void set_yaw_rate(float yaw_rate, mavlink_set_position_target_local_ned_t &sp);
*/

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

	// Heartbeat
	mavlink_heartbeat_t heartbeat;

	// System Status
	mavlink_sys_status_t sys_status;

	// Battery Status
	mavlink_battery_status_t battery_status;

	// Radio Status
	mavlink_radio_status_t radio_status;

	// Local Position
	mavlink_local_position_ned_t local_position_ned;

	// Global Position
	mavlink_global_position_int_t global_position_int;

	// Local Position Target
	mavlink_position_target_local_ned_t position_target_local_ned;

	// Global Position Target
	mavlink_position_target_global_int_t position_target_global_int;

	// HiRes IMU
	mavlink_highres_imu_t highres_imu;

	// Attitude
	mavlink_attitude_t attitude;

	// System Parameters?


	// Time Stamps
	Time_Stamps time_stamps;

	void
		reset_timestamps()
	{
		time_stamps.reset_timestamps();
	}

};


#define MAVLINK_BEGIN 0xFE
#define MAVLINK_HEADDER_LEN 3

struct MESSAGE
{
	int m_cmd;
	int m_iByte;
	int m_payloadLen;
	char m_pBuf[256];

};

class VehicleInterface
{
public:
	VehicleInterface();
	~VehicleInterface();

	int m_systemID;
	int m_autopilotID;
	int m_companionID;

//	Mavlink_Messages m_currentMessages;
//	mavlink_set_position_target_local_ned_t m_initialPosition;

	bool open(char* pSerialName);
	void close(void);

//	bool writeMessage(mavlink_message_t message);
	bool readMessages();
//	void handleMessage(mavlink_message_t message);

	void rc_overide(int numChannel, int* pChannels);
	void controlMode(int mode);

/*	void update_setpoint(mavlink_set_position_target_local_ned_t setpoint);
	void enable_offboard_control();
	void disable_offboard_control();
	*/

	MESSAGE m_recvMsg;


private:
	bool	m_bConnected;

	SerialPort *m_pSerialPort;
//	mavlink_set_position_target_local_ned_t current_setpoint;

	int toggle_offboard_control(bool flag);
	void write_setpoint();

	char m_pBuf[256];


};


