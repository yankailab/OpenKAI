#ifndef MOTORMX430_H
#define MOTORMX430_H

#include <fcntl.h>
#include <termios.h>
#define STDIN_FILENO 0

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>  //used for delay
#include <dynamixel_sdk/dynamixel_sdk.h>

//Control table address for MX430-W350 (ref. LabRobotArmOfficial.py)
#define ADDR_PRO_MODEL 						0
#define ADDR_PRO_OPERATING_MODE 			11
#define ADDR_PRO_CURRENT_LIMIT 				38
#define ADDR_PRO_ACCELERATION_LIMIT			40
#define ADDR_PRO_VELOCITY_LIMIT 			44
#define ADDR_PRO_TORQUE_ENABLE 				64
#define ADDR_PRO_POSITION_D_GAIN			80
#define ADDR_PRO_POSITION_I_GAIN			82
#define ADDR_PRO_POSITION_P_GAIN			84
#define ADDR_PRO_FEEDFORWARD_2nd_GAIN		88
#define ADDR_PRO_FEEDFORWARD_1st_GAIN		90
#define ADDR_PRO_GOAL_CURRENT				102
#define ADDR_PRO_GOAL_VELOCITY				104
#define ADDR_PRO_PROFILE_ACCELERATION		108
#define ADDR_PRO_PROFILE_VELOCITY			112
#define ADDR_PRO_GOAL_POSITION				116
#define ADDR_PRO_MOVING						122
#define ADDR_PRO_MOVING_STATUS				123
#define ADDR_PRO_PRESENT_CURRENT			126
#define ADDR_PRO_PRESENT_POSITION			132
//udpate 2019/02/08:
#define ADDR_PRO_DRIVE_MODE					10
#define ADDR_PRO_PROFILE_ACCELERATION_TIME	108
#define ADDR_PRO_PROFILE_TIME_SPAN			112

//Protocol Version
#define PROTOCOL_VERSION					2.0

//Default setting
#define BAUDRATE						57600
#define DEVICENAME						"/dev/ttyUSB0"
#define TORQUE_ENABLE					1
#define TORQUE_DISABLE					0
#define DXL_MINIMUN_POSITION_VALUE		0.0
#define DXL_MAXIMUN_POSITION_VALUE		4095.0
#define DXL_MOVING_STATUD_THRESHOLD		10

//Motor's limits
#define VELOCITY_LIMIT					500				//ref: LabRobotArmOfficial.py
#define ACCELERATION_LIMIT				32767
#define CURRENT_LIMIT					1193				//mA
#define MIN_MOTOR_ANGLE					0				//Use for mapping				
#define MAX_MOTOR_ANGLE					4095				//Use for mapping

//udpate 2019/02/08: switch motors control to time_based control / function trajectory generation depreciated
// setprofile(acc, velo) function can be rename
#define VELOCITY_BASED					0
#define TIME_BASED						4

class MotorXM430
{
private:
	//Motors
	uint8_t m_ID;
	uint32_t m_model_number;

	//Control
	uint8_t m_mode;
	int8_t m_operating_mode;
	int m_current_limit;
	int m_goal_current;
	uint8_t m_drivingmode;

	//Status
	uint32_t m_present_position;

	//Communication instance and variables
	dynamixel::PacketHandler* packetHandler;
	dynamixel::PortHandler* portHandler;
	int dxl_comm_result;
	uint8_t dxl_error;

public:
	//Constructor:
	MotorXM430();

	bool init(int ID, int operating_mode, int current_limit, int goal_current, const char* portName, int baudRate);

	//Functions:
	//GetID: return the motors ID. Input: Void. Output : int ID. Example: MotorXM430 motor; printf("ID: %d\n",motor.GetID());
	int GetID();
	//GetModelNumber(): return the model of the motor. (ex: model numer 1020 is the MX430). This function is used to read the current of the motors. 
	//As the first function call in the construstor, we used this function to check if the motor is turn ON or without failure. In case of failure, this function will terminate the program.
	uint16_t GetModelNumber();
	//IsMoving(): return the moving status of the motor.Used for motor synchronisation when several motors are moving.
	uint8_t IsMoving();
	//MovingStatus: printf the moving status of the motors. Used for debugging.
	void MovingStatus();
	//ReadCurrent(): return the signed current of the motor in motorUnit. 
	int16_t ReadCurrent();

	//MAP: return the mapped angle value into the other unit. Used for converting angle degree to angle motors.]
	//Inputs: angle is the value we want to convert. in_min/max: the minimal/maximal angle value in the 1fst unit. out_min/max: the minimal/maximal angle value in the 2nd unit.
	//Output: the mapped angle.
	//Example: MAP(m_present_position, MIN_MOTOR_ANGLE, MAX_MOTOR_ANGLE, 0.0, 360.0)
	float MAP(uint32_t angle, long in_min, long in_max, long out_min, long out_max);

	//TorqueON - TorqueOFF: (dis-)activate the torque inside the motor. Without torque the motor will not move.
	void TorqueON();
	void TorqueOFF();

	//ReadAngle: return the current angle of the motor in degree
	float ReadAngle();

	//Goto: set the goal position in the motor. The function do not currently check the validity of the input
	//Input: wanted position in degree float
	void Goto(float position);

	//SetOperatingMode: set the operating mode in the motor (Availiable mode: 0 (Current), 1 (Velocity), 3 (Position), 4 (Extended position), 5 (current-based position), 16 (PWM))
	//Input: wanted mode uint8_t
	void SetOperatingMode(uint8_t mode);
	//PrintOperationMode: printf in the _Console the operationmode. Used for debugging.
	void PrintOperatingMode();

	//SetPID: set the different parameters of the internal motor's PID controler.
	//Input the P / I / D value between 0-16383 uint16_t
	void SetPID(uint16_t P_gain, uint16_t I_gain, uint16_t D_gain);
	//PrintPID: printf the gain value of the motor's PID in the consol. Used for debugging.
	void PrintPID();

	//SetFFGain: set the different parameters of the internal motor's FeedForward controler.
	//Input the FF1 FF2 value between 0-16383 uint16_t
	void SetFFGain(uint16_t FF1Gain, uint16_t FF2Gain);
	//PrintFFGain: printf the gain value of the motor's FeedForward control in the consol. Used for debugging.
	void PrintFFGain();

	//SetProfile: set the acceleration and velocity profile of the motor. Used to tunned the motor behaviour and for motor synchronisation when several motors are moving.
	//Input: wanted velocity (RPM) and acceleration (Rev/min2) uint32_t. The function check if the wanted value are in the motor's limits ( XM430: VELOCITY_LIMIT 500 / ACCELERATION_LIMIT 32767)
	void SetProfile(uint32_t Velocity, uint32_t Acceleration);
	//PrintProfile: printf the Velocity and Acceleration value in the consol. Used for debugging.
	void PrintProfile();

	//SetCurrentLimit: set the maximun current (torque) output of the motor. Used for current-based position control (gripper)
	//Input is set in the define section CURRENT_LIMIT 1193
	void SetCurrentLimit();
	//PrintCurrentLimit: printf the value of the current limit (mA) in the _Console. Used for debugging.
	void PrintCurrentLimit();

	//SetGoalCurrent: set the goal current. Used for current-based position control
	//Input: wanted goal current mA uint16_t. The function check if the wanted goalcurrent is not exceeding the currentlimit
	void SetGoalCurrent(uint16_t GoalCurrent);
	//PrintGoalcurrent: printf the value of the goalcurrent (mA) in the _Console. Used for debugging.
	void PrintGoalCurrent();

	//## Update 2019/02/08
	//SetDriving: set the driving mode of the motor, between TIMED_BASED and VELOCITY_BASED driving mode.
	//Input: driving mode input (0: Velocity / 4: Time)
	void SetDrivingMode(uint8_t type);
	//PrintDrivingMode: print and return the current driving mode of the motor
	uint8_t PrintDrivingMode();
	//SetTimeProfile: set the acceleration profile of the motor. Ta is the acceleration time, and tf is the final time. 
	//Input: Ta, Tf in ms. If Ta = 0, the profile is rectangle / if Ta=0.5Tf, the profile is triangle. Please refer to Dynamixel documentation for more information
	void SetTimeProfile(uint32_t Ta, uint32_t Tf);
	//PrintTimeProfile: print the current Ta and Tf profile parameter of the motor
	void PrintTimeProfile();
};

#endif
