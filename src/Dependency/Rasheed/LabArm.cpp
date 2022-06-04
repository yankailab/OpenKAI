#include "LabArm.h"

LabArm::LabArm()
{
	a1 = 0.0;
	d1 = 0.0;
	alph1 = M_PI / 2.0;

	a2 = 212.0;
	d2 = 0.0;
	alph2 = 0.0;

	a3 = 74.0;
	d3 = 0.0;
	alph3 = M_PI / 2.0;

	a4 = 0.0;
	d4 = 139.0;
	alph4 = -M_PI / 2.0;

	a5 = 0.0;
	d5 = 0.0;
	alph5 = M_PI / 2.0;

	a6 = 0.0;
	d6 = 230.0;
	alph6 = 0.0;

	//Set other arm parameters:
	a_state = -1;
	a_gripper = 2;
	a_torque = 0;

	//Home:								//There is probably a betterway to do this...
	Home1[0] = M1_HOME_1;
	Home1[1] = M2_HOME_1;
	Home1[2] = M3_HOME_1;
	Home1[3] = M4_HOME_1;
	Home1[4] = M5_HOME_1;
	Home1[5] = M6_HOME_1;

	Home2[0] = M1_HOME_2;
	Home2[1] = M2_HOME_2;
	Home2[2] = M3_HOME_2;
	Home2[3] = M4_HOME_2;
	Home2[4] = M5_HOME_2;
	Home2[5] = M6_HOME_2;
}

bool LabArm::init(const char *portName, int baudRate)
{
	if (!portName)
		return false;

	if (!motor1.init(DXL1_ID, M1_MODE, M1_CURRENT_LIMIT, M1_GOAL_CURRENT, portName, baudRate))
		return false;
	if (!motor2.init(DXL2_ID, M2_MODE, M2_CURRENT_LIMIT, M2_GOAL_CURRENT, portName, baudRate))
		return false;
	if (!motor3.init(DXL3_ID, M3_MODE, M3_CURRENT_LIMIT, M3_GOAL_CURRENT, portName, baudRate))
		return false;
	if (!motor4.init(DXL4_ID, M4_MODE, M4_CURRENT_LIMIT, M4_GOAL_CURRENT, portName, baudRate))
		return false;
	if (!motor5.init(DXL5_ID, M5_MODE, M5_CURRENT_LIMIT, M5_GOAL_CURRENT, portName, baudRate))
		return false;
	if (!motor6.init(DXL6_ID, M6_MODE, M6_CURRENT_LIMIT, M6_GOAL_CURRENT, portName, baudRate))
		return false;
	if (!gripper.init(DXL7_ID, GRIPPER_MODE, GRIPPER_CURRENT_LIMIT, GRIPPER_GOAL_CURRENT, portName, baudRate))
		return false;

	return true;
}

//Kinetics functions
void LabArm::printMatrix(double matrix[][4], int size)
{
	//Print matrix: (square) // cut into 2 function ? add print line?
	for (int l = 0; l < size; l++)
	{
		for (int c = 0; c < size; c++)
		{
			printf("%f ", matrix[l][c]);
		}
		printf("\n");
	}
}

void LabArm::printMatrix3(double matrix[][3], int size)
{
	//Print matrix: (square) // cut into 2 function ? add print line?
	for (int l = 0; l < size; l++)
	{
		for (int c = 0; c < size; c++)
		{
			printf("%f ", matrix[l][c]);
		}
		printf("\n");
	}
}

void LabArm::multiplyMatrix(double m1[][4], double m2[][4], double m12[][4],
							int size)
{
	for (int l = 0; l < size; l++)
	{
		for (int c = 0; c < size; c++)
		{
			//Init:
			m12[l][c] = 0;
			//Calculation
			for (int k = 0; k < size; k++)
			{
				m12[l][c] += m1[l][k] * m2[k][c];
			}
		}
	}
}

void LabArm::multiplyMatrix3(double m1[][3], double m2[][3], double m12[][3],
							 int size)
{
	for (int l = 0; l < size; l++)
	{
		for (int c = 0; c < size; c++)
		{
			//Init:
			m12[l][c] = 0;
			//Calculation
			for (int k = 0; k < size; k++)
			{
				m12[l][c] += m1[l][k] * m2[k][c];
			}
		}
	}
}

void LabArm::Solve(double A[][2], double B[], double X[])
{
	//Only 2x2 matrix so no LU factorization needed -> direct Gaussian elimination.
	double k = A[1][0] / A[0][0];
	X[1] = (B[1] - k * B[0]) / (A[1][1] - k * A[0][1]);
	X[0] = (B[0] - A[0][1] * X[1]) / A[0][0];
}

void LabArm::RobotArmFWD(float motorAngle[], float positionGripper[]) //motorangle an array of the 6 motors position | positionGripper an array to return the XYZ and rotation of the gripper
{
	//Convertion
	double q1 = (motorAngle[0] - 90) * DEG2RAD;
	double q2 = (motorAngle[1] - 90) * DEG2RAD;
	double q3 = (motorAngle[2] - 180) * DEG2RAD;
	double q4 = (motorAngle[3] - 180) * DEG2RAD;
	double q5 = (motorAngle[4] - 180) * DEG2RAD;
	double q6 = (motorAngle[5] - 180) * DEG2RAD;

	//Matrix creation:
	double T01[4][4] =
		{
			{cos(q1), 0, sin(q1), a1 * cos(q1)},
			{sin(q1), 0, -cos(q1), a1 * cos(q1)},
			{0, 1, 0, 0},
			{0, 0, 0, 1}};

	double T12[4][4] =
		{
			{cos(q2), -sin(q2), 0, a2 * cos(q2)},
			{sin(q2), cos(q2), 0, a2 * sin(q2)},
			{0, 0, 1, 0},
			{0, 0, 0, 1}};

	double T23[4][4] =
		{
			{cos(q3), 0, sin(q3), a3 * cos(q3)},
			{sin(q3), 0, -cos(q3), a3 * sin(q3)},
			{0, 1, 0, 0},
			{0, 0, 0, 1}};

	double T34[4][4] =
		{
			{cos(q4), 0, -sin(q4), 0},
			{sin(q4), 0, cos(q4), 0},
			{0, -1, 0, d4},
			{0, 0, 0, 1}};

	double T45[4][4] =
		{
			{cos(q5), 0, sin(q5), 0},
			{sin(q5), 0, -cos(q5), 0},
			{0, 1, 0, 0},
			{0, 0, 0, 1}};

	double T56[4][4] =
		{
			{cos(q6), -sin(q6), 0, 0},
			{sin(q6), cos(q6), 0, 0},
			{0, 0, 1, d6},
			{0, 0, 0, 1}};

	double T02[4][4];
	double T03[4][4];
	double T04[4][4];
	double T05[4][4];
	double T06[4][4];
	multiplyMatrix(T01, T12, T02, 4);
	multiplyMatrix(T02, T23, T03, 4);
	multiplyMatrix(T03, T34, T04, 4);
	multiplyMatrix(T04, T45, T05, 4);
	multiplyMatrix(T05, T56, T06, 4);

	//Extract position:
	double qX = T06[0][3];
	double qY = T06[1][3];
	double qZ = T06[2][3];

	//Extract rotation:
	double R06[3][3] =
		{
			{T06[0][0], T06[0][1], T06[0][2]},
			{T06[1][0], T06[1][1], T06[1][2]},
			{T06[2][0], T06[2][1], T06[2][2]}};
	//printMatrix3(R06,3);

	// Calculate the inverse of RPY constant
	double INV_RPY_const[3][3] =
		{
			{0, 0, 1},
			{1, 0, 0},
			{0, 1, 0}};
	double TCP[3][3];
	multiplyMatrix3(INV_RPY_const, R06, TCP, 3);

	double x6_rot = atan2(TCP[2][1], TCP[2][2]) * RAD2DEG;
	double z6_rot = atan2(TCP[1][0], TCP[0][0]) * RAD2DEG;
	double y6_rot = atan2(-TCP[2][0],
						  ((cos(z6_rot * DEG2RAD) * TCP[0][0]) + (sin(z6_rot * DEG2RAD) * TCP[1][0]))) *
					RAD2DEG;

	//Output:
	positionGripper[0] = qX;
	positionGripper[1] = qY;
	positionGripper[2] = qZ;
	positionGripper[3] = x6_rot;
	positionGripper[4] = y6_rot;
	positionGripper[5] = z6_rot;
}

void LabArm::armINV(float wantedXYZ[], float outputMotorAngle[]) //Output in degree?
{
	double qX = wantedXYZ[0];
	double qY = wantedXYZ[1];
	double qZ = wantedXYZ[2];
	double x6_rot = wantedXYZ[3];
	double y6_rot = wantedXYZ[4];
	double z6_rot = wantedXYZ[5];

	//Conversion
	double rr = x6_rot * DEG2RAD;
	double pp = y6_rot * DEG2RAD;
	double yy = z6_rot * DEG2RAD;

	double TCP[3][3] =
		{
			{cos(yy) * cos(pp), sin(pp) * sin(rr) - cos(rr) * sin(yy), sin(rr) * sin(yy) + cos(yy) * sin(pp)},
			{cos(pp) * sin(yy), cos(rr) * cos(yy) + sin(rr) * sin(yy) * sin(pp), cos(rr) * sin(yy) * sin(pp) - cos(yy) * sin(rr)},
			{-sin(pp), cos(pp) * sin(rr), cos(rr) * cos(pp)}};
	double psi = 90 * DEG2RAD;
	double theta = -90 * DEG2RAD;
	double phi = 180 * DEG2RAD;
	double RPY[3][3] =
		{
			{cos(psi) * cos(theta), cos(psi) * sin(phi) * sin(theta) - cos(phi) * sin(psi), sin(phi) * sin(psi) + cos(phi) * cos(psi) * sin(theta)},
			{cos(theta) * sin(psi), cos(phi) * cos(psi) + sin(phi) * sin(psi) * sin(theta), cos(phi) * sin(psi) * sin(theta) - cos(psi) * sin(phi)},
			{-sin(theta), cos(theta) * sin(phi), cos(phi) * cos(theta)}};

	double RPY_TCP[3][3];
	multiplyMatrix3(RPY, TCP, RPY_TCP, 3);

	//Matrix decomposition:
	double ux = RPY_TCP[0][0];
	double uy = RPY_TCP[1][0];
	double uz = RPY_TCP[2][0];
	double vx = RPY_TCP[0][1];
	double vy = RPY_TCP[1][1];
	double vz = RPY_TCP[2][1];
	double wx = RPY_TCP[0][2];
	double wy = RPY_TCP[1][2];
	double wz = RPY_TCP[2][2];
	double px = qX - d6 * wx;
	double py = qY - d6 * wy;
	double pz = qZ - d6 * wz;
	//Find q1:
	double q1 = atan(py / px);
	double q1_1 = 0;
	double q1_2 = 0;
	if ((q1 < M_PI) && (q1 > 0))
	{
		q1_1 = q1;
		q1_2 = q1 + M_PI;
	}
	else
	{
		q1_1 = q1 + M_PI;
		q1_2 = q1;
		q1 = q1 + M_PI;
	}
	double deg1[2] =
		{q1_1 * RAD2DEG, q1_2 * RAD2DEG};
	//Find q3:
	double k1 = 2 * a2 * d4;
	double k2 = 2 * a2 * a3;
	double k3 = px * px + py * py + pz * pz - 2 * pz * a1 * cos(q1) - 2 * py * a1 * sin(q1) + a1 * a1 - a2 * a2 - a3 * a3 - d4 * d4;
	double q3_1 = 2 * atan((k1 + sqrt(k1 * k1 + k2 * k2 - k3 * k3)) / (k3 + k2));
	double q3_2 = 2 * atan((k1 - sqrt(k1 * k1 + k2 * k2 - k3 * k3)) / (k3 + k2));
	double deg3[2] =
		{q3_1 * RAD2DEG, q3_2 * RAD2DEG};
	//Find q2:
	double uu1_0 = a2 + a3 * cos(q3_1) + d4 * sin(q3_1);
	double vv1_0 = -a3 * sin(q3_1) + d4 * cos(q3_1);
	double yy1_0 = px * cos(q1) + py * sin(q1) - a1;
	double uu2_0 = a3 * sin(q3_1) - d4 * cos(q3_1);
	double vv2_0 = a2 + a3 * cos(q3_1) + d4 * sin(q3_1);
	double yy2_0 = pz;

	double uu1_1 = a2 + a3 * cos(q3_2) + d4 * sin(q3_2);
	double vv1_1 = -a3 * sin(q3_2) + d4 * cos(q3_2);
	double yy1_1 = px * cos(q1) + py * sin(q1) - a1;
	double uu2_1 = a3 * sin(q3_2) - d4 * cos(q3_2);
	double vv2_1 = a2 + a3 * cos(q3_2) + d4 * sin(q3_2);
	double yy2_1 = pz;

	double A0[2][2] =
		{
			{uu1_0, vv1_0},
			{uu2_0, vv2_0}};
	double B[2] =
		{yy1_0, yy2_0};
	double A1[2][2] =
		{
			{uu1_1, vv1_1},
			{uu2_1, vv2_1}};
	double B1[2] =
		{yy1_1, yy2_1};
	//Resolution:
	double X0[2] =
		{0, 0};
	double X1[2] =
		{0, 0};
	Solve(A0, B, X0);
	Solve(A1, B1, X1);
	double q2_1 = atan2(X0[1], X0[0]);
	double q2_2 = atan2(X1[1], X1[0]);
	double deg2[2] =
		{q2_1 * RAD2DEG, q2_2 * RAD2DEG};
	//Find q5:
	double r33[2] =
		{(wx * cos(q1) * sin(q2_1 + q3_1) + wy * sin(q1) * sin(q2_1 + q3_1) - wz * cos(q2_1 + q3_1)), (wx * cos(q1) * sin(q2_2 + q3_2) + wy * sin(q1) * sin(q2_2 + q3_2) - wz * cos(q2_2 + q3_2))};
	double q5_1 = acos(r33[0]);
	double q5_2 = acos(r33[1]);
	double deg5[2] =
		{q5_1 * RAD2DEG, q5_2 * RAD2DEG};
	//Find q4:
	double q4_1 = 0;
	double q4_2 = 0;
	if (abs(r33[1]) < 1)
	{
		double cq4_1 = (wx * cos(q1) * cos(q2_1 + q3_1) + wy * sin(q1) * cos(q2_1 + q3_1) + wz * sin(q2_1 + q3_1)) / sin(q5_1);
		double cq4_2 = (wx * cos(q1) * cos(q2_2 + q3_2) + wy * sin(q1) * cos(q2_2 + q3_2) + wz * sin(q2_2 + q3_2)) / sin(q5_2);
		double sq4_1 = (wx * sin(q1) - wy * cos(q1)) / sin(q5_1);
		double sq4_2 = (wx * sin(q1) - wy * cos(q1)) / sin(q5_2);
		q4_1 = atan2(sq4_1, cq4_1);
		q4_2 = atan2(sq4_2, cq4_2);
	}
	else if (r33[1] == 1)
	{
		q4_1 = 0;
		q4_2 = 0;
	}
	else
	{
		printf("R33 Configuration physically imposible -> software error\n");
	}
	double deg4[2] =
		{q4_1 * RAD2DEG, q4_2 * RAD2DEG};
	//Find q6:
	double q6_1 = 0;
	double q6_2 = 0;
	if (abs(r33[1]) < 1)
	{
		double cq6_1 = -(ux * cos(q1) * sin(q2_1 + q3_1) + uy * sin(q1) * sin(q2_1 + q3_1) - uz * cos(q2_1 + q3_1)) / sin(q5_1);
		double cq6_2 = -(ux * cos(q1) * sin(q2_2 + q3_2) + uy * sin(q1) * sin(q2_2 + q3_2) - uz * cos(q2_2 + q3_2)) / sin(q5_2);
		double sq6_1 = (vx * cos(q1) * sin(q2_1 + q3_1) + vy * sin(q1) * sin(q2_1 + q3_1) - vz * cos(q2_1 + q3_1)) / sin(q5_1);
		double sq6_2 = (vx * cos(q1) * sin(q2_2 + q3_2) + vy * sin(q1) * sin(q2_2 + q3_2) - vz * cos(q2_2 + q3_2)) / sin(q5_2);
		q6_1 = atan2(sq6_1, cq6_1);
		q6_2 = atan2(sq6_2, cq6_2);
	}
	else if (r33[1] == 1)
	{
		q6_1 = 0;
		q6_2 = 0;
	}
	else
	{
		printf(
			"abs(R33) Configuration physically imposible -> software error\n");
	}
	double deg6[2] =
		{q6_1 * RAD2DEG, q6_2 * RAD2DEG};

	outputMotorAngle[0] = deg1[0] + 90;
	outputMotorAngle[1] = deg2[1] + 90;
	outputMotorAngle[2] = deg3[1] + 180;
	outputMotorAngle[3] = deg4[1] + 180;
	outputMotorAngle[4] = deg5[1] + 180;
	outputMotorAngle[5] = deg6[1] + 180;
}

// ARM Functions:
void LabArm::MotorsInit(int mode)
{
	printf("ARM: Settings the motors (PID, FF)\n");
	switch (mode)
	{
	case 3:
		//Set same PID for every one
		printf("Setting mode 3\n");
		motor1.SetPID(PID_MODE3_P, PID_MODE3_I, PID_MODE3_D);
		motor1.SetFFGain(FFGAIN_MODE3_1st, FFGAIN_MODE3_2nd);
		motor2.SetPID(PID_MODE3_P, PID_MODE3_I, PID_MODE3_D);
		motor2.SetFFGain(FFGAIN_MODE3_1st, FFGAIN_MODE3_2nd);
		motor3.SetPID(PID_MODE3_P, PID_MODE3_I, PID_MODE3_D);
		motor3.SetFFGain(FFGAIN_MODE3_1st, FFGAIN_MODE3_2nd);
		motor4.SetPID(PID_MODE3_P, PID_MODE3_I, PID_MODE3_D);
		motor4.SetFFGain(FFGAIN_MODE3_1st, FFGAIN_MODE3_2nd);
		motor5.SetPID(PID_MODE3_P, PID_MODE3_I, PID_MODE3_D);
		motor5.SetFFGain(FFGAIN_MODE3_1st, FFGAIN_MODE3_2nd);
		motor6.SetPID(PID_MODE3_P, PID_MODE3_I, PID_MODE3_D);
		motor6.SetFFGain(FFGAIN_MODE3_1st, FFGAIN_MODE3_2nd);
		break;
	case 5:
		//Set specific PID for each motors
		printf("Setting mode 5\n");
		motor1.SetPID(M1_PID_MODE5_P, M1_PID_MODE5_I, M1_PID_MODE5_D);
		motor1.SetFFGain(FFGAIN_MODE5_1st, FFGAIN_MODE5_2nd);
		motor2.SetPID(M2_PID_MODE5_P, M2_PID_MODE5_I, M2_PID_MODE5_D);
		motor2.SetFFGain(FFGAIN_MODE5_1st, FFGAIN_MODE5_2nd);
		motor3.SetPID(M3_PID_MODE5_P, M3_PID_MODE5_I, M3_PID_MODE5_D);
		motor3.SetFFGain(FFGAIN_MODE5_1st, FFGAIN_MODE5_2nd);
		motor4.SetPID(M4_PID_MODE5_P, M4_PID_MODE5_I, M4_PID_MODE5_D);
		motor4.SetFFGain(FFGAIN_MODE5_1st, FFGAIN_MODE5_2nd);
		motor5.SetPID(M5_PID_MODE5_P, M5_PID_MODE5_I, M5_PID_MODE5_D);
		motor5.SetFFGain(FFGAIN_MODE5_1st, FFGAIN_MODE5_2nd);
		motor6.SetPID(M6_PID_MODE5_P, M6_PID_MODE5_I, M6_PID_MODE5_D);
		motor6.SetFFGain(FFGAIN_MODE5_1st, FFGAIN_MODE5_2nd);
		break;
	default:
		printf("Mode unknown recognized, PID are not set.\n");
		//Gripper initialization
		gripper.SetPID(GRIPPER_PID_P, GRIPPER_PID_I, GRIPPER_PID_D);
		gripper.SetProfile(GRIPPER_VSTD, GRIPPER_ASTD);
		printf("Lab ARM ready to operate\n");
	}
}

void LabArm::TorqueON()
{
	if (a_torque == 0)
	{
		motor1.TorqueON();
		motor2.TorqueON();
		motor3.TorqueON();
		motor4.TorqueON();
		motor5.TorqueON();
		motor6.TorqueON();
		printf("ARM TORQUE ON\n");
		a_torque = 1;
	}
}

void LabArm::TorqueOFF()
{
	if (a_torque == 1)
	{
		motor1.TorqueOFF();
		motor2.TorqueOFF();
		motor3.TorqueOFF();
		motor4.TorqueOFF();
		motor5.TorqueOFF();
		motor6.TorqueOFF();
		//gripper.TorqueOFF();
		printf("ARM TORQUE OFF\n");
		a_torque = 0;
	}
}

void LabArm::ReadArmCurrent(int motorCurrent[])
{
	motorCurrent[0] = motor1.ReadCurrent();
	motorCurrent[1] = motor2.ReadCurrent();
	motorCurrent[2] = motor3.ReadCurrent();
	motorCurrent[3] = motor4.ReadCurrent();
	motorCurrent[4] = motor5.ReadCurrent();
	motorCurrent[5] = motor6.ReadCurrent();
}

void LabArm::ReadAngle(float outputAngle[])
{
	outputAngle[0] = motor1.ReadAngle();
	outputAngle[1] = motor2.ReadAngle();
	outputAngle[2] = motor3.ReadAngle();
	outputAngle[3] = motor4.ReadAngle();
	outputAngle[4] = motor5.ReadAngle();
	outputAngle[5] = motor6.ReadAngle();
	//Uncomment for debugging
	for (int a = 0; a < 6; a++)
	{
		//printf("Angle motor %d = %f (degrees)\n",a+1, outputAngle[a]);
	}
}
void LabArm::RunArm(float inputAngle[])
{
	motor6.Goto(inputAngle[5]);
	motor5.Goto(inputAngle[4]);
	motor4.Goto(inputAngle[3]);
	motor3.Goto(inputAngle[2]);
	motor2.Goto(inputAngle[1]);
	motor1.Goto(inputAngle[0]);
	//Wait until all the motors finished to get to goal position (Ismoving return 1 if moving)
	int MovingFlag = -1;
	usleep(100000);
	while (MovingFlag != 0)
	{
		MovingFlag = motor1.IsMoving() + motor2.IsMoving() + motor3.IsMoving() + motor4.IsMoving() + motor5.IsMoving() + motor6.IsMoving();
	}
}

void LabArm::GoHome()
{
	//GoHOME in 2 steps
	TimeProfileGeneration(Home1, DEFAULT_TIMEACC, DEFAULT_TIMESPAN);
	RunArm(Home1);
	TimeProfileGeneration(Home2, DEFAULT_TIMEACC, DEFAULT_TIMESPAN);
	GripperOpen();
	RunArm(Home2);
	a_state = 0;
}

void LabArm::Awake()
{
	if (a_state > 0)
	{
		printf("Arm already awake\n");
	}
	else
	{
		TorqueON();
		//Move to Awake (home 2 -> 1)
		TimeProfileGeneration(Home2, DEFAULT_TIMEACC, DEFAULT_TIMESPAN);
		RunArm(Home2);
		TimeProfileGeneration(Home1, DEFAULT_TIMEACC, DEFAULT_TIMESPAN);
		RunArm(Home1);
		a_state = 1;
	}
}

void LabArm::StandBy()
{
	Awake();
	float StandbyPos[6] =
		{180, 180, 180, 180, 180, 180};
	TimeProfileGeneration(StandbyPos, DEFAULT_TIMEACC, DEFAULT_TIMESPAN);
	RunArm(StandbyPos);
	a_state = 2;
}

float LabArm::MAP(uint32_t angle, long in_min, long in_max, long out_min,
				  long out_max)
{
	return (((float)angle - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

float LabArm::DeltaPosition(float prePosition, float goalPosition) //Return the delta angle as motors angle difference
{
	if (prePosition > 360 || goalPosition > 360)
	{
		printf("Invalid position\n");
		return (0);
	}
	else
	{
		return ((int)abs(
			MAP(prePosition, 0, 360, MIN_MOTOR_ANGLE, MAX_MOTOR_ANGLE) - MAP(goalPosition, 0, 360, MIN_MOTOR_ANGLE,
																			 MAX_MOTOR_ANGLE)));
	}
}

int LabArm::FindMaxDelta(int deltaPosition[])
{
	int indexMax = 0;
	for (int a = 1; a < 6; a++)
	{
		if (deltaPosition[indexMax] < deltaPosition[a])
		{
			indexMax = a;
		}
	}
	return (indexMax);
}

void LabArm::TimeProfileGeneration(float goalPosition[], uint32_t stdTa,
								   uint32_t stdTf)
{
	float angleMotor[7];
	ReadAngle(angleMotor);
	int deltaPosition[6];

	//Calcul the delta position:
	for (int motor = 0; motor < 6; motor++)
	{
		deltaPosition[motor] = DeltaPosition(angleMotor[motor],
											 goalPosition[motor]);
	}

	//Find the bigger delta position and set as reference:
	int maxID = FindMaxDelta(deltaPosition);

	//Set the profile:
	motor1.SetTimeProfile(stdTa, stdTf);
	motor2.SetTimeProfile(stdTa, stdTf);
	motor3.SetTimeProfile(stdTa, stdTf);
	motor4.SetTimeProfile(stdTa, stdTf);
	motor5.SetTimeProfile(stdTa, stdTf);
	motor6.SetTimeProfile(stdTa, stdTf);
}

void LabArm::Goto(float goalPosition[], uint32_t stdTa, uint32_t stdTf)
{
	Awake();
	//Update: 2019/02/08
	TimeProfileGeneration(goalPosition, stdTa, stdTf);
	RunArm(goalPosition);
}

int LabArm::WorkSpaceLimitation(float X, float Y, float Z)
{
	float Qy = (RMAX - d6) * cos(150.0 * DEG2RAD);
	float Qz = (RMAX - d6) * sin(150.0 * DEG2RAD);
	float Sy = (RMAX - d6 - d4) * cos(150.0 * DEG2RAD);
	float Sz = (RMAX - d6 - d4) * sin(150.0 * DEG2RAD);

	if (((X * X + Y * Y + Z * Z) <= RMAX * RMAX) && ((X * X + Y * Y + Z * Z) >= R2MAX))
	{
		if ((Y < 0) && (Z > 0))
		{
			if (((X * X + (Y - Qy) * (Y - Qy) + (Z - Qz) * (Z - Qz)) > (d6 * d6)) && ((X * X + (Y - Sy) * (Y - Sy) + (Z - Sz) * (Z - Sz)) > (d6 * d6 + d4 * d4)))
			{
				return (0);
			}
			else
			{
				return (1);
			}
		}
		else if ((Y > 0) && (Z < 0))
		{
			if (Z > HMAX)
			{
				return (0);
			}
			else
			{
				return (2);
			}
		}
		else if ((Y > 0) && (Z > 0))
		{
			return (0);
		}
		else
		{
			return (3);
		}
	}
	else
	{
		return (4);
	}
}

int LabArm::WorkSpaceHorizontalLimitation(float X, float Y, float Z)
{
	float m3 = (HL_ZMIN_INNER - HL_ZMIN_OUTER) / (HL_YMIN - HL_YMAX);
	float c3 = HL_ZMIN_OUTER - m3 * HL_YMAX;
	float m4 = (HL_ZMAX_INNER - HL_MAX_OUTER) / (HL_YMIN - HL_YMAX);
	float c4 = HL_ZMAX_INNER - m4 * HL_YMIN;

	if ((Y >= 0) && (Z >= 0))
	{
		if ((Y >= HL_YMIN) && (Y <= HL_YMAX))
		{
			float Zc = m3 * Y + c3;
			float Z4 = m4 * Y + c4;
			float Rc = Z4 - Zc;
			if ((X * X + (Z - Zc) * (Z - Zc)) <= Rc * Rc)
			{
				return (0);
			}
			else
			{
				return (1);
			}
		}
		else
		{
			return (2);
		}
	}
	else
	{
		return (3);
	}
}

void LabArm::GetXYZ(float gripperPosition[])
{
	float motorAngle[7];
	ReadAngle(motorAngle);
	RobotArmFWD(motorAngle, gripperPosition);

	//printout:
	//	printf("position Gripper X: %f\n", gripperPosition[0]);
	//	printf("position Gripper Y: %f\n", gripperPosition[1]);
	//	printf("position Gripper Z: %f\n", gripperPosition[2]);
	//	printf("position Gripper rot X: %f\n", gripperPosition[3]);
	//	printf("position Gripper rot Y: %f\n", gripperPosition[4]);
	//	printf("position Gripper rot Z: %f\n", gripperPosition[5]);
}

void LabArm::GotoXYZ(float wantedXYZ[])
{
	//Get the INV kinetics:
	float inputMotorAngle[6];

	//checkworkplace:
	int checkWS = WorkSpaceLimitation(wantedXYZ[0], wantedXYZ[1], wantedXYZ[2]);
	if (checkWS == 0)
	{
		armINV(wantedXYZ, inputMotorAngle);
		for (int a = 0; a < 6; a++)
		{
			printf("Goal angle motor %d: %f\n", a, inputMotorAngle[a]);
		}
		Goto(inputMotorAngle, DEFAULT_TIMEACC, DEFAULT_TIMESPAN);
	}
	else
	{
		printf("XYZ are not in the global working space (error: %d)\n", checkWS);
	}
}

//GRIPPER FUNCTIONS
void LabArm::GripperON()
{
	gripper.TorqueON();
}

void LabArm::GripperOFF()
{
	gripper.TorqueOFF();
}

void LabArm::GripperOpen()
{
	if (a_gripper != 1)
	{
		printf("Opening the gripper\n");
		gripper.Goto(GRIPPER_OPEN);
		//Wait until all the motors finished to get to goal position (Ismoving return 1 if moving)
		int MovingFlag = -1;
		usleep(200000);
		while (MovingFlag != 0)
		{
			MovingFlag = gripper.IsMoving();
		}
		a_gripper = 1;
	}
	else
	{
		printf("Gripper already opened\n");
	}
}

void LabArm::GripperClose()
{
	if (a_gripper != 0)
	{
		printf("Closing the gripper\n");
		gripper.Goto(GRIPPER_CLOSE);
		//Wait until all the motors finished to get to goal position (Ismoving return 1 if moving)
		int MovingFlag = -1;
		usleep(200000);
		while (MovingFlag != 0)
		{
			MovingFlag = gripper.IsMoving();
		}
		a_gripper = 0;
	}
	else
	{
		printf("Gripper already closed\n");
	}
}

int LabArm::GripperGetCurrent()
{
	return (gripper.ReadCurrent());
}

float LabArm::GetSize()
{

	//Equation obtained by experimentation
	if (a_gripper == 0)
	{
		float angle = gripper.ReadAngle();
		//printf("Measured angle : %f\n",angle);
		float distance = -3.5364e-7 * pow(angle, 4) + 2.4396e-4 * pow(angle, 3) - 0.05430 * pow(angle, 2) + 4.8772 * angle - 154.0552;
		return (distance);
	}
	else
	{
		printf("The gripper didn't closed");
		return (-1);
	}
}

float LabArm::AverageCurrent(int n)
{
	float current = 0;
	for (int i = 0; i < n; i++)
	{
		current = current + motor5.ReadCurrent();
		sleep(0.5);
	}
	return (current / n);
}

float LabArm::Thoughness()
{
	float size1 = -1, size2 = -1, deformation = -1, aveDeformation = -1;
	for (int i = 0; i < 1; i++)
	{
		sleep(0.5);
		size1 = GetSize();
		sleep(0.5);
		gripper.SetGoalCurrent(400);
		sleep(1);
		size2 = GetSize();
		gripper.SetGoalCurrent(90);
		deformation = abs(size1 - size2);
		aveDeformation = aveDeformation + deformation;
	}
	return (aveDeformation / 1);
}

float LabArm::Weight()
{
	//Change Operating Mode
	/*float currentAngle=motor5.ReadAngle();
	 motor5.SetOperatingMode(5);
	 motor5.SetGoalCurrent(500);
	 motor5.SetPID(1200, 0, 0);
	 motor5.Goto(currentAngle); */
	usleep(500);

	//Weight position
	float WeightPosition5[6] =
		{180, 90, 180, 90, 270, 180};
	float current = 0, currentAmp = 0;
	float torque = 0;
	float weight = 0;
	float aveWeight = 0;
	sleep(0.2);
	Goto(WeightPosition5, DEFAULT_TIMEACC, DEFAULT_TIMESPAN);

	//Weight
	for (int i = 0; i < 3; i++)
	{
		//printf("Motor 5: Position %f\n",motor5.ReadAngle());
		current = AverageCurrent(10);
		//printf("Motor 5: Current (motorUnit): %f\n",current);
		currentAmp = 2.65e-3 * current;
		//printf("Current motor 5: %f A\n",currentAmp);
		torque = 1.75 * currentAmp + 0.15;
		//printf("Torque Motor 5: %f N.m\n", torque);
		weight = torque / (9.8 * 230e-3);
		//printf("Calculated Weight : %f (kg)\n",weight);
		aveWeight = aveWeight + weight;
		sleep(1);
	}
	printf("\nAveraged weight over 5 measurement: %f\n", aveWeight / 3);
	return (aveWeight / 3);
}

int LabArm::ObjectDetection()
{
	//Suppose the gripper is already close on the object with a goal current set at 90mA

	//Go though the trained SVM... predict(size1, abs(size1-size2))
	return (-1);
}

//Joystick control
int LabArm::FindSelectedMotor(uint8_t buttonstate[])
{
	//motor from 0 to 5
	int selectedmotor = -1;
	for (int m = 0; m < 6; m++)
	{
		if (buttonstate[m] == 1)
		{
			selectedmotor = m;
			m = 6;
		}
	}
	return (selectedmotor);
}

int LabArm::JoystickControl()
{
	Joystick joystick("/dev/input/js0");
	bool finish = false;
	int mode = 0;
	uint8_t buttonstate[10] =
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float axistate[10] =
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int selectedmotor = -1;
	float motorAngles[6];
	float angleIncrement = 12.0;
	bool openstate = true;

	//Set profile:
	motor1.SetProfile(40, 15);
	motor2.SetProfile(40, 15);
	motor3.SetProfile(40, 15);
	motor4.SetProfile(40, 15);
	motor5.SetProfile(40, 15);
	motor6.SetProfile(40, 15);

	if (!joystick.isFound())
	{
		printf(
			"No joystick found. Please check the connection or the path /dev/input/js0\n");
		return (1);
	}
	else
	{
		printf("\n Press start to start the arm control\n");
	}

	while (!finish)
	{
		usleep(1000);
		//Get only button even to make button priority
		JoystickEvent event;
		if ((joystick.sample(&event)) && !event.isInitialState())
		{
			if (event.isButton())
			{
				buttonstate[event.number] = event.value;
				//printf("Axis %d is %d\n",event.number, buttonstate[event.number]);
			}
		}

		//Mode 0: wait for start to awake the arm
		//Mode 1: select motor
		//Mode 2: read axis event and increment the motor angle.
		switch (mode)
		{
		case 0:
			buttonstate[6] == 1 ? finish = true : finish = false;
			if (buttonstate[7] == 1 || selectedmotor >= 0)
			{
				mode = 1;
				if (buttonstate[7] == 1)
				{
					printf("The arm is waking up.\n");
					StandBy();
					GripperON();
					//Set profile:
					motor1.SetProfile(40, 15);
					motor2.SetProfile(40, 15);
					motor3.SetProfile(40, 15);
					motor4.SetProfile(40, 15);
					motor5.SetProfile(40, 15);
					motor6.SetProfile(40, 15);
				}
				printf(
					"\n\n#####  How to Use  #####\nHold on the axis button, and use the analog left joystick (right-left).\n");
				printf(
					"Joint 1	-->	A\nJoint 2	-->	B\nJoint 3	-->	X\nJoint 4	-->	Y\nJoint 5	-->	LB\nJoint 6	-->	RB\n");
				printf(
					"Press the left analog joystick to use the gripper.\n\nYou can press the BACK button at anytime to stop the arm.\n\n");
				axistate[0] = 0;
				selectedmotor = -1;
			}
			break;

		case 1:
			buttonstate[6] == 1 ? finish = true : finish = false;
			selectedmotor = FindSelectedMotor(buttonstate);
			if (selectedmotor >= 0)
			{
				mode = 2;
				printf("The selected joint is %d\n", selectedmotor + 1);
				printf(
					"Move the left analog joystick in the right-left direction to move the joint.\nWhen finished just release the axis button.\n");
			}
			if (buttonstate[9] == 1)
			{
				if (openstate)
				{
					GripperClose();
					openstate = false;
				}
				else
				{
					GripperOpen();
					openstate = true;
				}
			}
			break;

		case 2:
			buttonstate[6] == 1 ? finish = true : finish = false;
			if (buttonstate[selectedmotor] == 0)
			{
				mode = 0;
				axistate[0] = 0;
				break;
			}

			//Get axis event
			JoystickEvent event;
			if ((joystick.sample(&event)) && !event.isInitialState())
			{
				if (event.isButton())
				{
					buttonstate[event.number] = event.value;
					//printf("Axis %d is %d\n",event.number, buttonstate[event.number]);
				}
				if (event.isAxis())
				{
					axistate[event.number] = event.value / AXISMAXVALUE;
					//printf("Axis %d is %f\n",event.number, axistate[event.number]);
				}
			}

			//Rechek the button status based on the new event
			buttonstate[6] == 1 ? finish = true : finish = false;
			if (buttonstate[selectedmotor] == 0)
			{
				mode = 0;
				axistate[0] = 0;
				break;
			}

			//update the angle
			ReadAngle(motorAngles);
			motorAngles[selectedmotor] = motorAngles[selectedmotor] + angleIncrement * axistate[0];
			//printf("Motor %d, new angle: %f\n",selectedmotor, motorAngles[selectedmotor]);

			//Run only one motor...
			switch (selectedmotor)
			{
			case 0:
				motor1.Goto(motorAngles[0]);
				break;
			case 1:
				motor2.Goto(motorAngles[1]);
				break;
			case 2:
				motor3.Goto(motorAngles[2]);
				break;
			case 3:
				motor4.Goto(motorAngles[3]);
				break;
			case 4:
				motor5.Goto(motorAngles[4]);
				break;
			case 5:
				motor6.Goto(motorAngles[5]);
				break;
			}

			break;
		}
	}

	printf("The arm is going Home.\n");
	GoHome();
	TorqueOFF();
	return (0);
}
