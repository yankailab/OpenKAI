#ifndef RASHEED_LABARM_H
#define RASHEED_LABARM_H

#ifdef USE_DYNAMIXEL

#include "motorMX430.h"	
#include "joystick.h"
#include <math.h>					
#include <chrono>

// Parameter for the different motors:   (ref. LabRobotArmOfficial (python))
//Motor 1:
#define DXL1_ID					1
#define M1_HOME_1				278.85
#define M1_HOME_2				285.45
#define M1_MODE					3
#define M1_CURRENT_LIMIT		800
#define M1_GOAL_CURRENT			150
#define M1_PID_MODE5_P			800
#define M1_PID_MODE5_I			100
#define M1_PID_MODE5_D			9000
//Motor 2:
#define DXL2_ID					2
#define M2_HOME_1				194.63
#define M2_HOME_2				175.47
#define M2_MODE					3
#define M2_CURRENT_LIMIT		1000
#define M2_GOAL_CURRENT			800
#define M2_PID_MODE5_P			800
#define M2_PID_MODE5_I			100
#define M2_PID_MODE5_D			9000
//Motor 3
#define DXL3_ID					3
#define M3_HOME_1				155.69
#define M3_HOME_2				103.82
#define M3_MODE					3
#define M3_CURRENT_LIMIT		800
#define M3_GOAL_CURRENT			250
#define M3_PID_MODE5_P			800
#define M3_PID_MODE5_I			100
#define M3_PID_MODE5_D			9000
//Motor 4
#define DXL4_ID					4
#define M4_HOME_1				183.48
#define M4_HOME_2				195.48
#define M4_MODE					3
#define M4_CURRENT_LIMIT		800
#define M4_GOAL_CURRENT			150
#define M4_PID_MODE5_P			800
#define M4_PID_MODE5_I			50
#define M4_PID_MODE5_D			7000
//Motor 5
#define DXL5_ID					5
#define M5_HOME_1				103.03
#define M5_HOME_2				174.41
#define M5_MODE					3
#define M5_CURRENT_LIMIT		800
#define M5_GOAL_CURRENT			250
#define M5_PID_MODE5_P			800
#define M5_PID_MODE5_I			50
#define M5_PID_MODE5_D			7000
//Motor 6
#define DXL6_ID					6
#define M6_HOME_1				198.89
#define M6_HOME_2				178.89
#define M6_MODE					3
#define M6_CURRENT_LIMIT		800
#define M6_GOAL_CURRENT			150
#define M6_PID_MODE5_P			800
#define M6_PID_MODE5_I			50
#define M6_PID_MODE5_D			7000
//Gripper
#define DXL7_ID					7
#define GRIPPER_HOME_1			150
#define GRIPPER_OPEN			297
#define GRIPPER_CLOSE			171
#define GRIPPER_MODE			5
#define GRIPPER_CURRENT_LIMIT	800
#define GRIPPER_GOAL_CURRENT	90
#define GRIPPER_PID_P			2000
#define GRIPPER_PID_I			30
#define GRIPPER_PID_D			2000
#define GRIPPER_ASTD			60
#define GRIPPER_VSTD			200
//Motors global parameters
#define PID_MODE3_P				1500
#define PID_MODE3_I				100
#define PID_MODE3_D				4000
#define FFGAIN_MODE3_1st		100
#define FFGAIN_MODE3_2nd		50
#define FFGAIN_MODE5_1st		300
#define FFGAIN_MODE5_2nd		50
//Workspace limitations
#define RMAX					600.0  //full extended raduis
#define R2MAX					75.0  //innner raduis, not to hit it self
#define HMAX					-200.0  // maybe change when on rover
#define HL_YMAX					450.0     //HL: Horizontal Limitation
#define HL_YMIN					230
#define HL_ZMIN_INNER			74.0
#define HL_ZMAX_INNER			380.0
#define HL_ZMIN_OUTER			0.0
#define HL_MAX_OUTER			250.0
//Set profile to slow
#define V_SLOW					40
#define A_SLOW					8 
//Constant
#define DEG2RAD					0.01745
#define RAD2DEG					57.2957
#define AXISMAXVALUE			32767.0

// Update 2019/02/08
#define DEFAULT_TIMEACC			500 //ms
#define DEFAULT_TIMESPAN		2000

class LabArm
{
public:
	// Motor list:
	MotorXM430 motor1;
	MotorXM430 motor2;
	MotorXM430 motor3;
	MotorXM430 motor4;
	MotorXM430 motor5;
	MotorXM430 motor6;
	MotorXM430 gripper;

private:
	//D.H parameters:
	double a1;
	double d1;
	double alph1;

	double a2;
	double d2;
	double alph2;

	double a3;
	double d3;
	double alph3;

	double a4;
	double d4;
	double alph4;

	double a5;
	double d5;
	double alph5;

	double a6;
	double d6;
	double alph6;

	//Other parameters
	int8_t a_state; 					// 0=home, 1=awake, 2=standby
	uint8_t a_gripper; 				//0=closed, 1=opened
	uint8_t a_torque;					//0=torqueOff, 1=torqueON
	float ArmAngle[7];
	//Home coordonates
	float Home1[6];
	float Home2[6];

public:
	LabArm();

	bool init(const char* portName, int baudRate);

	//###################################################################  KINEMATICS FUNCTIONS
	//printMatrix*: printf matrix contain in the _Console. Used for debugging.
	//Input: matrix[4][4] or [3][3] depending on the function. (can be optimized into one function)
	void printMatrix(double matrix[][4], int size);
	void printMatrix3(double matrix[][3], int size);

	//multiplyMatrix*: multiply 2 matrix. Used for INV-Foward kinetics function.
	//Input: m1, m2 the 2 matrix to multiply. m12 the matrix result (passed by reference) (can be optimized into one function) 
	void multiplyMatrix(double m1[][4], double m2[][4], double m12[][4], int size);
	void multiplyMatrix3(double m1[][3], double m2[][3], double m12[][3], int size);

	//Solve: solve a Ax = b linear system. Used for INV-Forward kinetics function. (direct resultion by Gaussian Elimination)
	//Input: the A matrix system [2][2], b [2] array and X [2] array result (passed by reference)
	void Solve(double A[][2], double B[], double X[]);

	//RobotArmFWD: calcul the forward kinematics of the arm. Used to get the XYZ and rotation of the gripper, 
	//Input: motorAngle is an [6] float array which contain the 6 motors current position (degree), positionGripper[6] the float array of the result (passed by reference)
	void RobotArmFWD(float motorAngle[], float positionGripper[]);

	//armINV: calcul the inverse kinematic of the arm. Used for moving the arm based on XYZ command.
	//Input: wantedXYZ positon (mm | degree) for the gripper float array [6], outputMotor the [6] array which contained the motors angle (degree)
	void armINV(float wantedXYZ[], float outputMotorAngle[]);

	//###################################################################  ARM MOVEMENT FUNCTIONS
	//Init: set all the parameteres of the 7 motors depending on the wanted mode. The profile are all set to 0 (step motion).
	//Input: wanted mode (Availiable mode: 3 (Position), 5 (current-based position). (othermode are not supported 0 (Current), 1 (Velocity),  4 (Extended position), 16 (PWM))
	void MotorsInit(int mode);
	//TorqueON - TorqueOFF: (dis-)activate the torque inside all the 6 arm's motor. Without torque the arm will not move.
	void TorqueON();
	void TorqueOFF();

	//ReadArmCurrent: return all motor's current.Used for arm monitoring. The current value will be printed automatically, if unwanted delete the corresponding printf section in motorXM430.cpp
	//Input: int array [6] passed by reference to get the current value for each motors.
	void ReadArmCurrent(int motorCurrent[]);
	//ReadAngle: write the current angle of each motors (degree) in the passed array. 
	//Input: float [6] array to contain the 6 motors angles.
	void ReadAngle(float outputAngle[]);

	//RunArm: write the goal position in the 6 motors. This function make the arm move, and wait until all motors stop moving.
	//Input: inputAngle array[6]  contain the wanted angle for each motors in degree.
	void RunArm(float inputAngle[]);

	//MAP: return the mapped angle value into the other unit. Used for converting angle degree to angle motors.]
	//Inputs: angle is the value we want to convert. in_min/max: the minimal/maximal angle value in the 1fst unit. out_min/max: the minimal/maximal angle value in the 2nd unit.
	//Output: the mapped angle.
	//Example: MAP(m_present_position, MIN_MOTOR_ANGLE, MAX_MOTOR_ANGLE, 0.0, 360.0)
	float MAP(uint32_t angle, long in_min, long in_max, long out_min,
			long out_max);

	//GoHome(): make the arm go to the home position (home1 -> home2). The function used the Home1 and Home2[ ] float array constitued of all the MOTOR_*_HOME* of the #define section.
	void GoHome();
	//Awake(): make the arm go to the awake position (home2 -> home1). The function used the Home1 and Home2[ ] float array constitued of all the MOTOR_*_HOME* of the #define section.
	void Awake();
	//StandBy: make the arm go to the Standby position. For now the standby position is hardcoded {180, 180, 180, 180, 180, 180}  (maybe better to create a choice for the user.
	void StandBy();

	//DeltaPosition: calcul the angle difference between the prePosition and the goal position. Used for trajectory calculation. (maybe more optimal to pass an array into that function)
	//Input: the prePosition of the motor (usually the current position) and the goal position in degree.
	//Output: the differences between the angle (degree) 
	float DeltaPosition(float prePosition, float goalPosition);
	//FindMaxDelta: search and return the array index of the biggest deltaposition. Used for trajectory calculation.
	//Input: the deltaPosition[6] array which containt the differences between the the currentPosition and the goalposition
	//Output: the array index corresponding to the bigges deltaposition.
	int FindMaxDelta(int deltaPosition[]);

	//TimeProfileGeneration: set the timeprofile of the arm. Used to time the smooth movement of the arm. Currently constant, but maybe implemented as prop of total angular distance
	//Input: the goal position array[6] containing the wanted position of the 6 motors, the based velocity-acceleration for each motors
	void TimeProfileGeneration(float goalPosition[], uint32_t stdTa,
			uint32_t stdTf);

	//Goto: make the arm go to the wanted position. If generateTrajectory is 1, the generated movement will be smooth, if generateTrajectory is 0, all the motors will have the same profile (Vstd, Astd)
	//Input: the goal position array[6] containing the wanted position of the 6 motors, the option for smooth movement, the standart profile for the motors.
	void Goto(float goalPosition[], uint32_t stdTa, uint32_t stdTf);

	//WorkSpaceLimitation: check if the wanted X,Y,Z position are in the working space of the arm. The workingspace parameter are fixed in the #define section
	//Ouput: return 0: XYZ are in the working space /  return 1: out of work range in quadrant 2 or 3 / return 2:  Z is lower than the lowest range HMAX / return 3: // Y, Z axe is negatif / return4: X,Y,Z point is not in not in the workspace
	int WorkSpaceLimitation(float X, float Y, float Z);

	//WorkSpaceHorizontalLimitation: check if the wanted X,Y,Z position are in the horizontal working space of the arm. The workingspace parameter are fixed in  the #define section. Used to keep a camera horizontal.
	//Ouput: return 0: XYZ are in the working space / return 1: Input out of the XZ plane / return 2: Y out of range / return 3: one of the input if negatif.
	int WorkSpaceHorizontalLimitation(float X, float Y, float Z);

	//GetXYZ: calcul the inverve kinematics of the arm and write the result in passed array.
	//Input: gripperPosition array[6] result of the calculation. The array is construted as gripperPosition[ * ] = {X, Y, Z, x6_rot, y6_rot, z6_rot}
	void GetXYZ(float gripperPosition[]);

	//GotoXYZ: make the arm go to the wanted XYZ position. The arm moves only if the XYZ are in the working space
	//Input the wantedXYZ array contening the wanted gripperPosition {X, Y, Z, x6_rot, y6_rot, z6_rot}.
	void GotoXYZ(float wantedXYZ[]);

	//###################################################################  GRIPPER FUNCTIONS
	//GripperON - OFF: (dis-)activate the torque in gripper's motor. Without torque the gripper will not close or open.
	void GripperON();
	void GripperOFF();
	//GripperOpen-close: make the gripper close or open. The function check the status of the gripper to see if it is already in the wanted state. The goal position for the close-open can be modified in the #define section.
	void GripperOpen();
	void GripperClose();
	//GripperGetCurrent: read the current inside the gripper's motor. Not used yet but can be use for object size and/or object material recognition.
	int GripperGetCurrent();

	float GetSize();
	//GetSize(): return the size of the gripped object based on experimental calibration
	//Output: float size in mm.

	float AverageCurrent(int n);
	//AverageCurrent: return the current in the motor5 averaged on N measurment. Used for weight evaluation.
	//The float current in Motor's Unit.

	float Weight();
	//Weight: move the arm into the weight positon and estimat the weight of the grabbed object. (This function is still underdevelopment)
	//Output: the averaged weight of the object over 5 measurment.

	float Thoughness();
	//Toughness: try to estimate the thoughness of the grabbed object. (This function is still underdevelopment)
	//Output: the averaged estimated thoughness over 5 measurment.	

	int ObjectDetection();
	//ObjectDetection: try to predict the type of grabbed object based on the size, toughness and weight though a linear SVM classifier. (This function is still underdevelopment)
	//Output the class of the object. The function assume the object is already grabbed.

	//###################################################################  JOYSTICK FUNCTIONS
	//FindSelectedMotor: return the id corresponding to a high state of the array buttonstate. Used in JoystickControl to find the join to move.
	//Input: the buttonstate array contening updated state of the joystick button
	int FindSelectedMotor(uint8_t buttonstate[]);

	//JoystickControl: allow you to control the arm with a joystick. The control is done 1 join at the time.  (This function code is overcomplicated due to issue on reading event from the joystick (axis event oversampled))
	//To move a join you need to keep pressed the angle corresponding to the join (see table) and move the left joystick right or left.
	int JoystickControl();

};

#endif
#endif
