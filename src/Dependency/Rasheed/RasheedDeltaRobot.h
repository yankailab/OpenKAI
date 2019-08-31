#ifndef DELTAROBOTCLASS_H
#define DELTAROBOTCLASS_H

#include "../../Config/switch.h"
#ifdef USE_DYNAMIXEL

#if defined(__linux__) || defined(__APPLE__)
#include <fcntl.h>
#include <termios.h>
#define STDIN_FILENO 0
#elif defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>					
#include <chrono>
using namespace std::chrono;
#include <unistd.h>  //used for delay

#include <dynamixel_sdk/dynamixel_sdk.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Control table of XM540 and XM430 ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ADDR_PRO_MODEL 					    0
#define ADDR_PRO_DRIVE_MODE					10
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
#define ADDR_PRO_MOVING 					122
#define ADDR_PRO_MOVING_STATUS				123
#define ADDR_PRO_PRESENT_CURRENT			126
#define ADDR_PRO_PRESENT_POSITION			132
//TIME BASED PROFILE
#define ADDR_PRO_PROFILE_ACCELERATION_TIME	108
#define ADDR_PRO_PROFILE_TIME_SPAN			112

// Data Byte Length for Sync read/write
#define LEN_PRO_GOAL_POSITION           4
#define LEN_PRO_PRESENT_POSITION        4

//Protocol Version
#define PROTOCOL_VERSION				2.0

//Default setting
#define BAUDRATE						57600
#define DEVICENAME						"/dev/ttyUSB0"  
#define TORQUE_ENABLE					1			// ON
#define TORQUE_DISABLE					0			// OFF
#define DXL_MINIMUN_POSITION_VALUE		0			// Raw value (equivalent to 0 deg.)
#define DXL_MAXIMUN_POSITION_VALUE		4095		// Raw value (equivalent to 360 deg.)
#define DXL_MOVING_STATUD_THRESHOLD		10

//Motor's limits
#define VELOCITY_LIMIT					500			// ref.from Dynamixel SDK
#define ACCELERATION_LIMIT				32767		// ref. from Dynamixel SDK
#define CURRENT_LIMIT					2047		// Raw value 2047 for XM540 and 1193 for XM430
#define MIN_MOTOR_ANGLE					0			// Use for mapping to degree			
#define MAX_MOTOR_ANGLE					4095		// Use for mapping to degree

#define VELOCITY_BASED					0
#define TIME_BASED						4

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Parameters of Servo and Robot //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Motor 1:
#define DXL1_ID							1			// Dynamixel ID
#define M1_HOME							180			// Home angle of servo, in degree
#define M1_MODE							3			// 3:Position Control (default), 5:Current Based Position Control
#define M1_CURRENT_LIMIT				2047		// Max value is 2047 for XM540 and 1193 for XM430
#define M1_GOAL_CURRENT					2000		// In case of MODE is 5, if the current reach this value the motion will stop
//Motor 2:
#define DXL2_ID							2			// Dynamixel ID
#define M2_HOME_1						180			// Home angle of servo, in degree
#define M2_MODE							3			// 3:Position Control (default), 5:Current Based Position Control
#define M2_CURRENT_LIMIT				2047		// Max value is 2047 for XM540 and 1193 for XM430
#define M2_GOAL_CURRENT					2000		// In case of MODE is 5, if the current reach this value the motion will stop
//Motor 3
#define DXL3_ID							3			// Dynamixel ID
#define M3_HOME_1						180			// Home angle of servo [degree]
#define M3_MODE							3			// 3:Position Control (default), 5:Current Based Position Control
#define M3_CURRENT_LIMIT				2047		// Max value is 2047 for XM540 and 1193 for XM430
#define M3_GOAL_CURRENT					2000		// In case of MODE is 5, if the current reach this value the motion will stop
//Gripper
#define DXL4_ID							4			// Dynamixel ID
#define GRIPPER_OPEN					2350		// Fully Open value [Raw value]
#define GRIPPER_CLOSE					1840		// Fully Close value [Raw value], please adjust this value due to the object size
#define GRIPPER_MODE					5			// Always set as 5 (Current Based Position Control)			
#define GRIPPER_CURRENT_LIMIT			1193		// limit of current [Raw value]
#define GRIPPER_GOAL_CURRENT			500			// This will set how hard the gripper can grab [Raw value]

//Workspace limitations, refer to the working range diagram for more detail.
#define maxR							500.0 		// A radius of full working range circle	[mm]
#define minR 							300.0		// A minimum radius when full stoke 		[mm]
#define minStoke						-350.0		// minimum stoke that the robot can go up   [mm]
#define midStoke						-850.0		// max stoke when working radius is maxR 	[mm] 
#define maxStoke						-1000.0 	// maximum stoke that the robot can go down [mm]
#define homeHeight						-551.5149	// with this height and x=0,y=0, all of the proximal arm will strecth out parallel to the ground (or 180deg for all servo) [mm]

//Math Constant
#define DEG2RAD							0.01745
#define RAD2DEG							57.2957
#define AXISMAXVALUE					32767.0
#define ROOT3							1.7320508
#define PI 								3.141592654

//Special Parameters
#define OFFSET_ANGLE					3.8427      // This offset is for DELTA ROBOT MK2 only due to the joint offset between
// proximal and distal link, it will be used in FWD and INV

class RasheedDeltaRobot
{

private:

	//Servos Communication instance and variables
	dynamixel::PacketHandler *packetHandler;
	dynamixel::PortHandler *portHandler;
	dynamixel::GroupSyncWrite groupSyncWrite;
	dynamixel::GroupSyncRead groupSyncRead;
	int dxl_comm_result;
	uint8_t dxl_error;
	bool dxl_addparam_result = false;                 // addParam result
	bool dxl_getdata_result = false;                  // GetParam result

	//Robot's parameters [mm]:
	double sb;
	double sp;
	double L;
	double l;
	double wb;
	double ub;
	double wp;
	double up;

	//P I D gain in [Raw value]
	uint16_t PID_MODE3_P;
	uint16_t PID_MODE3_I;
	uint16_t PID_MODE3_D;
	uint16_t GRIPPER_PID_P;
	uint16_t GRIPPER_PID_I;
	uint16_t GRIPPER_PID_D;

	//Feedforward gain [Raw value]
	uint16_t FFGAIN_MODE3_1st;
	uint16_t FFGAIN_MODE3_2nd;		// Feedforward gain ref. from DYNAMIXEL SDK
	//uint16_t FFGAIN_MODE5_1st
	//uint16_t FFGAIN_MODE5_2nd

	//Operation speed of gripper and robot
	uint32_t GRIPPER_Ta;				// Acceleration time to open/close [ms]
	uint32_t GRIPPER_Tf;					// Finish time to open/close [ms]
	uint32_t DEFAULT_TIMEACC;			// Acceleration time to open/close [ms]
	uint32_t DEFAULT_TIMESPAN;				// Finish time to open/close [ms]

	// For RobotInit()
	uint8_t M1_operating_mode;
	uint8_t M2_operating_mode;
	uint8_t M3_operating_mode;
	uint8_t M1_drivingmode;
	uint8_t M2_drivingmode;
	uint8_t M3_drivingmode;
	uint32_t M1_model_number;
	uint32_t M2_model_number;
	uint32_t M3_model_number;

public:
	RasheedDeltaRobot();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////// Math and Kinematics function ///////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//MAP: return the mapped angle value into the other unit. Used for converting angle degree to angle motors.]
	//Inputs: angle is the value we want to convert. 
	//in_min/max: the minimal/maximal angle value in the 1st unit. 
	//out_min/max: the minimal/maximal angle value in the 2nd unit.
	//Output: the mapped angle.
	//Example: MAP(m_present_position, MIN_MOTOR_ANGLE, MAX_MOTOR_ANGLE, 0.0, 360.0)
	float MAP(uint32_t angle, long in_min, long in_max, long out_min,
			long out_max);

	//printMatrix*: printf matrix contain in the console. Used for debugging.
	//Input: matrix[4][4] or [3][3] depending on the function. (can be optimized into one function)
	void printMatrix(double matrix[][4], int size);
	void printMatrix3(double matrix[][3], int size);

	//multiplyMatrix*: multiply 2 matrix. Used for INV-Foward kinetics function.
	//Input: m1, m2 the 2 matrix to multiply. m12 the matrix result (passed by reference) (can be optimized into one function) 
	void multiplyMatrix(double m1[][4], double m2[][4], double m12[][4],
			int size);
	void multiplyMatrix3(double m1[][3], double m2[][3], double m12[][3],
			int size);

	//FWD: calcul the forward kinematics of the arm. Used to get the XYZ and rotation of the gripper, 
	//Input: robotAngle is an [3] float array which contain the 3 motors current position (degree) accoding to kinematics model, 
	// positionXYZ[3] the float array of the result (passed by reference)
	void FWD(float robotAngle[], float positionXYZ[]);

	//INV: calcul the inverse kinematic of the arm. Used for moving the arm based on XYZ command.
	//Input: wantedXYZ positon (mm | degree) for the gripper float array [3], 
	// outputRobotAngle the [3] array which contained the motors angle (degree)
	void INV(float wantedXYZ[], float outputRobotAngle[]);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////// SERVO FUNCTIONS /////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//GetModelNumber(): return the model of the motor. (ex: model numer 1020 is the XM430). This function is used to read the current of the motors. 
	//As the first function call in the construstor, we used this function to check if the motor is turn ON or without failure. 
	//In case of failure, this function will terminate the program.
	//Input: Servo ID
	uint16_t GetModelNumber(int ID);

	//SetOperatingMode: set the operating mode in the motor 
	//Availiable mode: 0 (Current), 
	//                 1 (Velocity), 
	//                 3 (Position), 
	//                 4 (Extended position), 
	//                 5 (Current-based position)
	//				  16 (PWM)
	//Input: Servo ID, wanted mode uint8_t
	void SetOperatingMode(int ID, uint8_t mode);
	//PrintOperationMode: printf in the console the operationmode. Used for debugging.
	void PrintOperatingMode(int ID);

	//SetDriving: set the driving mode of the motor, between TIMED_BASED and VELOCITY_BASED driving mode.
	//Input: Servo ID, driving mode input (0: Velocity / 4: Time)
	void SetDrivingMode(int ID, uint8_t type);
	//PrintDrivingMode: print and return the current driving mode of the motor
	uint8_t PrintDrivingMode(int ID);

	//IsMoving: return the moving status of the motor.
	//Used for motor synchronisation when several motors are moving.
	//Input: Servo ID
	uint8_t IsMoving(int ID);

	//MovingStatus: printf the moving status of the motors. Used for debugging.
	//Input: Servo ID
	void MovingStatus(int ID);

	//SetPID: set the different parameters of the internal motor's PID controler.
	//Input: Servo ID,  the P / I / D value between 0-16383 uint16_t
	void SetPID(int ID, uint16_t P_gain, uint16_t I_gain, uint16_t D_gain);
	//PrintPID: printf the gain value of the motor's PID in the consol. Used for debugging.
	void PrintPID(int ID);

	//SetFFGain: set the different parameters of the internal motor's FeedForward controller.
	//Input: Servo ID, the FF1 FF2 value between 0-16383 uint16_t
	void SetFFGain(int ID, uint16_t FF1Gain, uint16_t FF2Gain);
	//PrintFFGain: printf the gain value of the motor's FeedForward control in the consol. Used for debugging.
	void PrintFFGain(int ID);

	//SetProfile: set the acceleration and velocity profile of the motor when DrivingMode is in velocity based control
	//Choose either SetProfile or SetTimeProfile.
	//Input: Servo ID, wanted velocity (RPM) and acceleration (Rev/min2) uint32_t. 
	//The function check if the wanted value are in the motor's limits 
	//( XM430: VELOCITY_LIMIT 500 / ACCELERATION_LIMIT 32767)
	void SetProfile(int ID, uint32_t Velocity, uint32_t Acceleration);
	//PrintProfile: printf the Velocity and Acceleration value in the consol. Used for debugging.
	void PrintProfile(int ID);

	//SetTimeProfile: set the acceleration profile of the motor when DrivingMode is in velocity based control. 
	//Ta is the acceleration time, and Tf is the final time. Choose either SetProfile or SetTimeProfile.
	//Input: Servo ID, Ta, Tf in ms. If Ta = 0, the profile is rectangle / if Ta=0.5Tf, the profile is triangle. 
	//Please refer to Dynamixel documentation for more information
	void SetTimeProfile(int ID, uint32_t Ta, uint32_t Tf);
	//PrintTimeProfile: print the current Ta and Tf profile parameter of the motor
	void PrintTimeProfile(int ID);

	//SetCurrentLimit: set the maximun current (torque) output of the motor. Used for current-based position control (gripper)
	//Input: Servo ID, is set in the define section CURRENT_LIMIT 1193
	void SetCurrentLimit(int ID, uint32_t current_limit);
	//PrintCurrentLimit: printf the value of the current limit (mA) in the console. Used for debugging.
	void PrintCurrentLimit(int ID);

	//SetGoalCurrent: set the goal current. Used for current-based position control
	//Input: Servo ID, wanted goal current mA uint16_t. The function check if the wanted goalcurrent is not exceeding the currentlimit
	void SetGoalCurrent(int ID, uint16_t GoalCurrent);
	//PrintGoalcurrent: printf the value of the goalcurrent (mA) in the console. Used for debugging.
	void PrintGoalCurrent(int ID);

	//TorqueON - TorqueOFF: (dis-)activate the torque inside the motor. Without torque the motor will not move.
	//Input: Servo ID,
	void TorqueON(int ID);
	void TorqueOFF(int ID);

	//ReadAngle: return the current angle of the motor in degree
	//Input: Servo ID,
	//float ReadAngle(int ID); 			//REMARKS// There is some problem with this function, still not sure the problem, but it may conflict with the SyncReadAngle() function...

	//DriveAngle: set the goal position for a single motor. The function do not currently check the validity of the input
	//Input: Servo ID, wanted position in degree float
	void DriveAngle(int ID, float deg);

	//SyncDriveAngle: set the goal position in degree for all of the servo almost in the same time
	//Input: position of each servo in degree float
	void SyncDriveAngle(float Deg123[]);

	//SyncReadAngle: read all of the servo angle almost in the same time
	//(passed by reference)
	void SyncReadAngle(float allAngle[]);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////// ROBOT MOVEMENT FUNCTIONS //////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//RobotInit: set all the parameteres of the 4 motors depending on the wanted mode. The profile are all set to 0 (step motion).
	//Input: wanted mode (Availiable mode: 3 (Position), 5 (current-based position). 
	//(othermode are not supported 0 (Current), 1 (Velocity),  4 (Extended position), 16 (PWM))
	void RobotInit(int mode);

	//RobotTorqueON - RobotTorqueOFF: (dis-)activate the torque of three sevos for the arm
	void RobotTorqueON();
	void RobotTorqueOFF();

	//GripperTorqueON - OFF: (dis-)activate the torque of gripper
	void GripperTorqueON();
	void GripperTorqueOFF();

	//GripperOpen-close: make the gripper close or open. The function check the status of the gripper to see if it is already in the wanted state. The goal position for the close-open can be modified in the #define section.
	void GripperOpen();
	void GripperClose();

	//GripperCheck: open and close gripper itself to check there is nothing left inside
	void GripperCheck();

	//Check the point of XYZ before passing to the GotoPoint whether the goal point is in the working space or not.
	//Input: XYZ coordinate from the base frame, return 0 for in the working range and 1 for out of working range.
	int XYZOutRange(float X, float Y, float Z);

	//IsAllStop: check whether the robot is finish the motion or not
	//if the robot was done, the code will come out from the while loop inside 
	void IsAllStop();

	//GotoPoint: drive to robot to the goal point in working space
	//Input: XYZ coordinate from the base frame,
	//WARNING - Z value must always be negative, due to configuration of this robot, the gripper is pointing down to the ground  
	void GotoPoint(float X, float Y, float Z);

	//GetXYZ: get the end effector coordinate in cartesian space, the program will do the forward kinematics from current position
	//(passed by reference)
	void GetXYZ(float gripperPosition[]);

	//GetServoAngle: get the angle data of all servos, this will use SyncReadAngle directly
	//(passed by reference)
	void GetRobotAngle(float RobotAngle[]);

	//GoHome(): let the robot go to home position as define from M1_HOME, M2_HOME. M3_HOME
	void GoHome();

	//KinematicsCheck: Use when debug the kinematics of robot
	void KinematicsCheck();
};

#endif
#endif
