#include "../Rasheed/RasheedDeltaRobot.h"

#ifdef USE_DYNAMIXEL

// Create a constructor and initialize portHandler, packetHandler, SyncRead/Write immediately before enter the function							
RasheedDeltaRobot::RasheedDeltaRobot() :
		portHandler(dynamixel::PortHandler::getPortHandler(DEVICENAME)), packetHandler(
				dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION)), groupSyncWrite(
				portHandler, packetHandler, ADDR_PRO_GOAL_POSITION,
				LEN_PRO_GOAL_POSITION), groupSyncRead(portHandler,
				packetHandler, ADDR_PRO_PRESENT_POSITION,
				LEN_PRO_PRESENT_POSITION)

{

	// Open port, ref. from DYNAMIXEL SDK Example
	if (portHandler->openPort())
	{
		printf("Succeeded to open the port!\n");
	}
	else
	{
		printf("Failed to open the port!\n");
	}

	// Set port baudrate
	if (portHandler->setBaudRate(BAUDRATE))
	{
		printf("Succeeded to change the baudrate!\n");
	}
	else
	{
		printf("Failed to change the baudrate!\n");
	}

	//Robot's parameters [mm]:
	sb = 311.769;
	sp = 87.134;
	L = 399.147;
	l = 683.5;
	wb = (ROOT3 / 6) * sb;
	ub = (ROOT3 / 3) * sb;
	wp = (ROOT3 / 6) * sp;
	up = (ROOT3 / 3) * sp;

	//P I D gain in [Raw value]
	PID_MODE3_P = 2000;
	PID_MODE3_I = 1000;
	PID_MODE3_D = 4000;

	//Feedforward gain [Raw value]
	FFGAIN_MODE3_1st = 150;	// please check on Dynamixel emanual for more detail
	FFGAIN_MODE3_2nd = 50;

	//Operation speed of gripper and robot [msec]
	DEFAULT_TIMEACC = 1000;	// This value tell the acceraletion/decerelation time of the robot (MUST lower than DEFAULT_TIMESPAN) [msec]
	DEFAULT_TIMESPAN = 2000;// This value tell how long does the robot need to take to complete motion in [msec]

	GRIPPER_Ta = 350;			// Similar as DEFAULT_TIMEACC, but for gripper
	GRIPPER_Tf = 500;			// Similar as DEFAULT_TIMESPAN, but for gripper

	GRIPPER_PID_P = 1000;
	GRIPPER_PID_I = 100;
	GRIPPER_PID_D = 4000;
	;

}
;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Math and Kinematics function ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float RasheedDeltaRobot::MAP(uint32_t angle, long in_min, long in_max,
		long out_min, long out_max)
{
	return (((float) angle - in_min) * (out_max - out_min) / (in_max - in_min)
			+ out_min);
}
;

void RasheedDeltaRobot::printMatrix(double matrix[][4], int size)
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
;

void RasheedDeltaRobot::printMatrix3(double matrix[][3], int size)
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
;

void RasheedDeltaRobot::multiplyMatrix(double m1[][4], double m2[][4],
		double m12[][4], int size)
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
;

void RasheedDeltaRobot::multiplyMatrix3(double m1[][3], double m2[][3],
		double m12[][3], int size)
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
;

void RasheedDeltaRobot::FWD(float robotAngle[], float positionXYZ[])
{
	//Convertion
	// RobotAngle is from servo data, but the real robot kinematics angle need to add offset angle
	double deg1 = (robotAngle[0] + OFFSET_ANGLE) * DEG2RAD;
	double deg2 = (robotAngle[1] + OFFSET_ANGLE) * DEG2RAD;
	double deg3 = (robotAngle[2] + OFFSET_ANGLE) * DEG2RAD;

	//Matrix creation:
	double A1v[3] =
	{ 0, -wb - L * cos(deg1) + up, -L * sin(deg1) };
	double A2v[3] =
	{ (ROOT3 / 2) * (wb + L * cos(deg2)) - sp / 2, 0.5 * (wb + L * cos(deg2))
			- wp, -L * sin(deg2) };
	double A3v[3] =
	{ (-ROOT3 / 2) * (wb + L * cos(deg3)) + sp / 2, 0.5 * (wb + L * cos(deg3))
			- wp, -L * sin(deg3) };

	double r1 = l;
	double r2 = l;
	double r3 = l;

	double x1 = A1v[0];
	double y1 = A1v[1];
	double z1 = A1v[2];

	double x2 = A2v[0];
	double y2 = A2v[1];
	double z2 = A2v[2];

	double x3 = A3v[0];
	double y3 = A3v[1];
	double z3 = A3v[2];

	int method;

	// parameters for computation
	double a;
	double b;
	double c;
	double d;
	double e;
	double f;
	double zn;
	double x[2];
	double y[2];
	double z[2];
	double a11;
	double a12;
	double a13;
	double a21;
	double a22;
	double a23;
	double b1;
	double b2;
	double a1;
	double a2;
	double a3;
	double a4;
	double a5;
	double a6;
	double a7;
	double numX;
	double denX;
	//double A;
	double B;
	double C;
	double realANS[3] =
	{ 0, 0, 0 };

	if ((z1 == z2) && (z2 == z3) && (z1 == z3))
	{
		method = 1;
	}
	else if ((z1 != z3) && (z2 != z3))
	{
		method = 2;
	}
	else if ((z1 != z2) && (z1 != z3))
	{
		method = 3;
	}
	else
	{
		method = 4;
	}
	switch (method)
	{
	case 1:
		zn = z1;
		a = 2 * (x3 - x1);
		b = 2 * (y3 - y1);
		c = pow(r1, 2) - pow(r3, 2.0) - pow(x1, 2.0) - pow(y1, 2.0)
				+ pow(x3, 2.0) + pow(y3, 2.0);
		d = 2 * (x3 - x2);
		e = 2 * (y3 - y2);
		f = pow(r2, 2) - pow(r3, 2.0) - pow(x2, 2.0) - pow(y2, 2.0)
				+ pow(x3, 2.0) + pow(y3, 2.0);
		numX = (c * e) - (b * f);
		denX = (a * e) - (b * d);
		x[0] = numX / denX;
		if (x[0] < 0.000001)
		{
			y[0] = 0;
		}
		//A = 1;
		B = -2 * zn;
		C = pow(zn, 2) - pow(r1, 2) + pow((x[0] - x1), 2) + pow((y[0] - y1), 2);
		z[0] = (-B + sqrt(pow(B, 2) - 4 * C) / 2);
		z[1] = (-B - sqrt(pow(B, 2) - 4 * C) / 2);

		if (z[0] < 0)
		{
			realANS[0] = x[0];
			realANS[1] = y[0];
			realANS[2] = z[0];
		}
		else if (z[1] < 0)
		{
			realANS[0] = x[0];
			realANS[1] = y[0];
			realANS[2] = z[1];
		}
		else
		{
			printf("FWD: In method 1, height z is zero \n");
		}
		break;

	case 2:
		a11 = 2 * (x3 - x1);
		a12 = 2 * (y3 - y1);
		a13 = 2 * (z3 - z1);

		a21 = 2 * (x3 - x2);
		a22 = 2 * (y3 - y2);
		a23 = 2 * (z3 - z2);

		b1 = pow(r1, 2) - pow(r3, 2) - pow(x1, 2) - pow(y1, 2) - pow(z1, 2)
				+ pow(x3, 2) + pow(y3, 2) + pow(z3, 2);
		b2 = pow(r2, 2) - pow(r3, 2) - pow(x2, 2) - pow(y2, 2) - pow(z2, 2)
				+ pow(x3, 2) + pow(y3, 2) + pow(z3, 2);

		a1 = (a11 / a13) - (a21 / a23);
		a2 = (a12 / a13) - (a22 / a23);
		a3 = (b2 / a23) - (b1 / a13);

		a4 = -a2 / a1;
		a5 = -a3 / a1;
		a6 = (-a21 * a4 - a22) / a23;
		a7 = (b2 - a21 * a5) / a23;

		a = pow(a4, 2) + 1 + pow(a6, 2);
		b = 2 * a4 * (a5 - x1) - 2 * y1 + 2 * a6 * (a7 - z1);
		c = a5 * (a5 - 2 * x1) + a7 * (a7 - 2 * z1) + pow(x1, 2) + pow(y1, 2)
				+ pow(z1, 2) - pow(r1, 2);

		y[0] = (-b + sqrt(pow(b, 2) - 4 * a * c)) / (2 * a);
		y[1] = (-b - sqrt(pow(b, 2) - 4 * a * c)) / (2 * a);

		x[0] = a4 * y[0] + a5;
		x[1] = a4 * y[1] + a5;
		z[0] = a6 * y[0] + a7;
		z[1] = a6 * y[1] + a7;

		if (z[0] < 0)
		{
			realANS[0] = x[0];
			realANS[1] = y[0];
			realANS[2] = z[0];
		}
		else if (z[1] < 0)
		{
			realANS[0] = x[1];
			realANS[1] = y[1];
			realANS[2] = z[1];
		}
		else
		{
			printf("FWD: In method 2, height z is zero \n");
		}
		break;

	case 3:
		a11 = 2 * (x1 - x2);
		a12 = 2 * (y1 - y2);
		a13 = 2 * (z1 - z2);

		a21 = 2 * (x1 - x3);
		a22 = 2 * (y1 - y3);
		a23 = 2 * (z1 - z3);

		b1 = pow(r2, 2) - pow(r1, 2) - pow(x2, 2) - pow(y2, 2) - pow(z2, 2)
				+ pow(x1, 2) + pow(y1, 2) + pow(z1, 2);
		b2 = pow(r3, 2) - pow(r1, 2) - pow(x3, 2) - pow(y3, 2) - pow(z3, 2)
				+ pow(x1, 2) + pow(y1, 2) + pow(z1, 2);

		a1 = (a11 / a13) - (a21 / a23);
		a2 = (a12 / a13) - (a22 / a23);
		a3 = (b2 / a23) - (b1 / a13);

		a4 = -a2 / a1;
		a5 = -a3 / a1;
		a6 = (-a21 * a4 - a22) / a23;
		a7 = (b2 - a21 * a5) / a23;

		a = pow(a4, 2) + 1 + pow(a6, 2);
		b = 2 * a4 * (a5 - x1) - 2 * y1 + 2 * a6 * (a7 - z1);
		c = a5 * (a5 - 2 * x1) + a7 * (a7 - 2 * z1) + pow(x1, 2) + pow(y1, 2)
				+ pow(z1, 2) - pow(r1, 2);

		y[0] = (-b + sqrt(pow(b, 2) - 4 * a * c)) / (2 * a);
		y[1] = (-b - sqrt(pow(b, 2) - 4 * a * c)) / (2 * a);

		x[0] = a4 * y[0] + a5;
		x[1] = a4 * y[1] + a5;
		z[0] = a6 * y[0] + a7;
		z[1] = a6 * y[1] + a7;

		if (z[0] < 0)
		{
			realANS[0] = x[0];
			realANS[1] = y[0];
			realANS[2] = z[0];
		}
		else if (z[1] < 0)
		{
			realANS[0] = x[1];
			realANS[1] = y[1];
			realANS[2] = z[1];
		}
		else
		{
			printf("FWD: In method 3, height z is zero \n");
		}
		break;
	case 4:
		a11 = 2 * (x2 - x1);
		a12 = 2 * (y2 - y1);
		a13 = 2 * (z2 - z1);

		a21 = 2 * (x2 - x3);
		a22 = 2 * (y2 - y3);
		a23 = 2 * (z2 - z3);

		b1 = pow(r1, 2) - pow(r2, 2) - pow(x1, 2) - pow(y1, 2) - pow(z1, 2)
				+ pow(x2, 2) + pow(y2, 2) + pow(z2, 2);
		b2 = pow(r3, 2) - pow(r2, 2) - pow(x3, 2) - pow(y3, 2) - pow(z3, 2)
				+ pow(x2, 2) + pow(y2, 2) + pow(z2, 2);

		a1 = (a11 / a13) - (a21 / a23);
		a2 = (a12 / a13) - (a22 / a23);
		a3 = (b2 / a23) - (b1 / a13);

		a4 = -a2 / a1;
		a5 = -a3 / a1;
		a6 = (-a21 * a4 - a22) / a23;
		a7 = (b2 - a21 * a5) / a23;

		a = pow(a4, 2) + 1 + pow(a6, 2);
		b = 2 * a4 * (a5 - x1) - 2 * y1 + 2 * a6 * (a7 - z1);
		c = a5 * (a5 - 2 * x1) + a7 * (a7 - 2 * z1) + pow(x1, 2) + pow(y1, 2)
				+ pow(z1, 2) - pow(r1, 2);

		y[0] = (-b + sqrt(pow(b, 2) - 4 * a * c)) / (2 * a);
		y[1] = (-b - sqrt(pow(b, 2) - 4 * a * c)) / (2 * a);

		x[0] = a4 * y[0] + a5;
		x[1] = a4 * y[1] + a5;
		z[0] = a6 * y[0] + a7;
		z[1] = a6 * y[1] + a7;

		if (z[0] < 0)
		{
			realANS[0] = x[0];
			realANS[1] = y[0];
			realANS[2] = z[0];
		}
		else if (z[1] < 0)
		{
			realANS[0] = x[1];
			realANS[1] = y[1];
			realANS[2] = z[1];
		}
		else
		{
			printf("FWD: In method 4, height z is zero \n");
		}
		break;

	}

	positionXYZ[0] = realANS[0];
	positionXYZ[1] = realANS[1];
	positionXYZ[2] = realANS[2];
}

void RasheedDeltaRobot::INV(float wantedXYZ[], float outputRobotAngle[])
{
	double x = wantedXYZ[0];
	double y = wantedXYZ[1];
	double z = wantedXYZ[2];

	double workingR = sqrt(pow(x, 2) + pow(y, 2));
	double a = wb - up;
	double b = sp / 2 - (ROOT3 / 2) * wb;
	double c = wp - 0.5 * wb;

	double E1 = 2 * L * (y + a);
	double F1 = 2 * z * L;
	double G1 = pow(x, 2) + pow(y, 2) + pow(z, 2) + pow(a, 2) + pow(L, 2)
			+ 2 * y * a - pow(l, 2);

	double E2 = -L * (ROOT3 * (x + b) + y + c);
	double F2 = 2 * z * L;
	double G2 = pow(x, 2) + pow(y, 2) + pow(z, 2) + pow(b, 2) + pow(c, 2)
			+ pow(L, 2) + 2 * (x * b + y * c) - pow(l, 2);

	double E3 = L * (ROOT3 * (x - b) - y - c);
	double F3 = 2 * z * L;
	double G3 = pow(x, 2) + pow(y, 2) + pow(z, 2) + pow(b, 2) + pow(c, 2)
			+ pow(L, 2) + 2 * (-x * b + y * c) - pow(l, 2);

	double t1[2];
	double t2[2];
	double t3[2];

	t1[0] = (-F1 + sqrt(pow(E1, 2) + pow(F1, 2) - pow(G1, 2))) / (G1 - E1);
	t1[1] = (-F1 - sqrt(pow(E1, 2) + pow(F1, 2) - pow(G1, 2))) / (G1 - E1);
	t2[0] = (-F2 + sqrt(pow(E2, 2) + pow(F2, 2) - pow(G2, 2))) / (G2 - E2);
	t2[1] = (-F2 - sqrt(pow(E2, 2) + pow(F2, 2) - pow(G2, 2))) / (G2 - E2);
	t3[0] = (-F3 + sqrt(pow(E3, 2) + pow(F3, 2) - pow(G3, 2))) / (G3 - E3);
	t3[1] = (-F3 - sqrt(pow(E3, 2) + pow(F3, 2) - pow(G3, 2))) / (G3 - E3);

	double theta1_1 = 2 * atan(t1[1]);
	double theta2_1 = 2 * atan(t2[1]);
	double theta3_1 = 2 * atan(t3[1]);

	double deg1 = theta1_1 * RAD2DEG;
	double deg2 = theta2_1 * RAD2DEG;
	double deg3 = theta3_1 * RAD2DEG;
	//printf("INV workingR: %f \n", workingR);
	//printf("INV z: %f \n", z);
	//printf("INV maxStoke: %f \n", maxStoke);
	//printf("INV minStoke: %f \n", minStoke);

	if ((workingR <= maxR) && (z > maxStoke) && (z <= minStoke))
	{

		// we remove the offset angle out before passing to output to let the servo drive to correct value
		outputRobotAngle[0] = deg1 - OFFSET_ANGLE;
		outputRobotAngle[1] = deg2 - OFFSET_ANGLE;
		outputRobotAngle[2] = deg3 - OFFSET_ANGLE;
	}
	else
	{
		printf("INV: Out of working range!...\n");
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////// SERVO FUNCTIONS /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint16_t RasheedDeltaRobot::GetModelNumber(int ID)
{
	uint16_t modelNumber = 0;
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, ID,
			ADDR_PRO_MODEL, &modelNumber);
	if (dxl_comm_result != COMM_SUCCESS)
	{
		printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
		usleep(1000000);
		printf("\n\n#######\nNot able to communicate with the motors\n");
		printf("Likely cause: no power / motor failure\n#######\n ");
		usleep(1000000);
		printf("Program exit()...\n");
		exit(EXIT_FAILURE);
	}
	return (modelNumber);
}

void RasheedDeltaRobot::SetOperatingMode(int ID, uint8_t mode)
{
	TorqueOFF(ID);

	if ((mode == 0) || (mode == 1) || (mode == 3) || (mode == 4) || (mode == 5)
			|| (mode == 16))
	{
		dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, ID,
				ADDR_PRO_OPERATING_MODE, mode, &dxl_error);
		if (dxl_comm_result != COMM_SUCCESS)
		{
			printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
		}
		else if (dxl_error != 0)
		{
			printf("%s\n", packetHandler->getRxPacketError(dxl_error));
		}
		printf("Motor %d is in ", ID);
		PrintOperatingMode(ID);
	}
	else
	{
		printf("Invalid Control Mode (Availiable mode: 0, 1, 3, 4, 5, 16)\n");
	}

}

void RasheedDeltaRobot::PrintOperatingMode(int ID)
{
	uint8_t mode = 3;
	dxl_comm_result = packetHandler->read1ByteTxRx(portHandler, ID,
			ADDR_PRO_OPERATING_MODE, &mode, &dxl_error);
	switch (mode)
	{
	case 0:
		printf("Operation Mode 0 : Current Control Mode\n");
		break;
	case 1:
		printf("Operation Mode 1 : Velocity Control Mode\n");
		break;
	case 3:
		printf("Operation Mode 3 : Position Control Mode\n");
		break;
	case 4:
		printf(
				"Operation Mode 4: Extented Position Control Mode (Multi-turn)\n");
		break;
	case 5:
		printf("Operation Mode 5: Current-base Position Control Mode\n");
		break;
	case 16:
		printf("Operation Mode 16: PWM Control Mode\n");
		break;
	default:
		printf("Invalid Control Mode (Availiable mode: 0, 1, 3, 4, 5, 16)\n");
	}
}

void RasheedDeltaRobot::SetDrivingMode(int ID, uint8_t type)
{
	dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, ID,
			ADDR_PRO_DRIVE_MODE, type);
	PrintDrivingMode(ID);
}

uint8_t RasheedDeltaRobot::PrintDrivingMode(int ID)
{
	uint8_t type;
	dxl_comm_result = packetHandler->read1ByteTxRx(portHandler, ID,
			ADDR_PRO_DRIVE_MODE, &type);
	printf("Motor %d is on driving mode: %d\n", ID, type);
	return (type);
}

uint8_t RasheedDeltaRobot::IsMoving(int ID)
{
	uint8_t moving;
	dxl_comm_result = packetHandler->read1ByteTxRx(portHandler, ID,
			ADDR_PRO_MOVING, &moving);
	return (moving);
}

void RasheedDeltaRobot::MovingStatus(int ID)
{
	uint8_t movingstatus = 0;
	dxl_comm_result = packetHandler->read1ByteTxRx(portHandler, ID,
			ADDR_PRO_MOVING_STATUS, &movingstatus);
	if (movingstatus > 48)
	{
		printf("Motor %d is in Trapezodal Profile\n", ID);
	}
	else if ((movingstatus < 35) && (movingstatus > 20))
	{
		printf("Motor %d is in Triangular Profile\n", ID);
	}
	else if (movingstatus > 3)
	{
		printf("Motor %d is in Rectangular Profile\n", ID);
	}
	else if (movingstatus >= 0)
	{
		printf("Motor %d is in Step Profile (No Profile)", ID);
	}
	else
	{
		printf("Motor %d UNKNOWN Profile\n", ID);
	}
}

void RasheedDeltaRobot::SetPID(int ID, uint16_t P_gain, uint16_t I_gain,
		uint16_t D_gain)
{
	dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, ID,
			ADDR_PRO_POSITION_P_GAIN, P_gain, &dxl_error);
	dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, ID,
			ADDR_PRO_POSITION_I_GAIN, I_gain, &dxl_error);
	dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, ID,
			ADDR_PRO_POSITION_D_GAIN, D_gain, &dxl_error);
	// check if PID are set:
	PrintPID(ID);
}
void RasheedDeltaRobot::PrintPID(int ID)
{
	uint16_t P, I, D;
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, ID,
			ADDR_PRO_POSITION_P_GAIN, &P, &dxl_error);
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, ID,
			ADDR_PRO_POSITION_I_GAIN, &I, &dxl_error);
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, ID,
			ADDR_PRO_POSITION_D_GAIN, &D, &dxl_error);
	printf("Motor %d: PID values : P: %d / I: %d / D: %d\n", ID, P, I, D);
}

void RasheedDeltaRobot::SetFFGain(int ID, uint16_t FF1Gain, uint16_t FF2Gain)
{
	dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, ID,
			ADDR_PRO_FEEDFORWARD_1st_GAIN, FF1Gain);
	dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, ID,
			ADDR_PRO_FEEDFORWARD_2nd_GAIN, FF2Gain);
	// check if FFgain are set:
	PrintFFGain(ID);
}
void RasheedDeltaRobot::PrintFFGain(int ID)
{
	uint16_t FF1Gain, FF2Gain;
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, ID,
			ADDR_PRO_FEEDFORWARD_1st_GAIN, &FF1Gain);
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, ID,
			ADDR_PRO_FEEDFORWARD_2nd_GAIN, &FF2Gain);
	printf("Motor %d, Feed forward Gain: FF1: %d / FF2: %d\n", ID, FF1Gain,
			FF2Gain);
}

void RasheedDeltaRobot::SetProfile(int ID, uint32_t Velocity,
		uint32_t Acceleration)
{
	//Set the limits:
	dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, ID,
			ADDR_PRO_VELOCITY_LIMIT, VELOCITY_LIMIT);
	dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, ID,
			ADDR_PRO_ACCELERATION_LIMIT, ACCELERATION_LIMIT);
	if (Velocity <= VELOCITY_LIMIT)
	{
		dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, ID,
				ADDR_PRO_PROFILE_VELOCITY, Velocity);
	}
	else
	{
		printf("Velocity out of range (limit=%d)\n", VELOCITY_LIMIT);
	}
	if (Acceleration <= ACCELERATION_LIMIT)
	{
		dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, ID,
				ADDR_PRO_PROFILE_ACCELERATION, Acceleration);
	}
	else
	{
		printf("Acceleration out of range (limit=%d)\n", ACCELERATION_LIMIT);
	}
}
void RasheedDeltaRobot::PrintProfile(int ID)
{
	uint32_t acceleration = 0;
	uint32_t velocity = 0;
	dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, ID,
			ADDR_PRO_PROFILE_VELOCITY, &velocity);
	dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, ID,
			ADDR_PRO_PROFILE_ACCELERATION, &acceleration);
	printf("Motor %d:	Acceleration Profile: %d and Velocity Profile: %d\n", ID,
			acceleration, velocity);
}

void RasheedDeltaRobot::SetTimeProfile(int ID, uint32_t Ta, uint32_t Tf)
{
	dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, ID,
			ADDR_PRO_PROFILE_ACCELERATION_TIME, Ta);
	dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, ID,
			ADDR_PRO_PROFILE_TIME_SPAN, Tf);
	PrintTimeProfile(ID);
}

void RasheedDeltaRobot::PrintTimeProfile(int ID)
{
	uint32_t Ti, Tf;
	dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, ID,
			ADDR_PRO_PROFILE_ACCELERATION_TIME, &Ti);
	dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, ID,
			ADDR_PRO_PROFILE_TIME_SPAN, &Tf);
	printf("Motor %d, acceleration time: %d / timespan: %d\n", ID, Ti, Tf);
}

void RasheedDeltaRobot::SetCurrentLimit(int ID, uint32_t current_limit)
{
	dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, ID,
			ADDR_PRO_CURRENT_LIMIT, current_limit);
}
void RasheedDeltaRobot::PrintCurrentLimit(int ID)
{
	float SetUnit = 1;
	uint16_t current_limit = CURRENT_LIMIT;
	float current_limit_amp = 0;
	float UnitMX106 = 3.96;
	float UnitXM540 = 2.69;
	float UnitXM430 = 2.69;
	int model_number;
	model_number = GetModelNumber(ID);
	switch (model_number)
	{
	case 321:
		SetUnit = UnitMX106;
		break;
	case 1020:
		SetUnit = UnitXM430;
		break;
	case 1120:
		SetUnit = UnitXM540;
		break;
	default:
		printf("Motor %d, model not supported.\n", ID);
	}
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, ID,
			ADDR_PRO_CURRENT_LIMIT, &current_limit);
	current_limit_amp = (float) current_limit * (SetUnit / 1000.0);
	printf("Motor %d, Current limit: %d (input motor), %f (amp)\n", ID,
			current_limit, current_limit_amp);
}

void RasheedDeltaRobot::SetGoalCurrent(int ID, uint16_t GoalCurrent)
{
	if ((GoalCurrent >= -CURRENT_LIMIT) || (GoalCurrent <= CURRENT_LIMIT))
	{
		dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, ID,
				ADDR_PRO_GOAL_CURRENT, GoalCurrent);
	}
	else
	{
		printf("Goal current out of range (limit=%d)\n", CURRENT_LIMIT);
	}
}
void RasheedDeltaRobot::PrintGoalCurrent(int ID)
{
	uint16_t goalCurrent = 0;
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, ID,
			ADDR_PRO_GOAL_CURRENT, &goalCurrent);
	printf("Motor %d: Goal Current: %d mA\n", ID, goalCurrent);
}

void RasheedDeltaRobot::TorqueON(int ID)
{
	dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, ID,
			ADDR_PRO_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);

	if (dxl_comm_result != COMM_SUCCESS)
	{
		printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
	}
	else if (dxl_error != 0)
	{
		printf("%s\n", packetHandler->getRxPacketError(dxl_error));
	}

}

void RasheedDeltaRobot::TorqueOFF(int ID)
{
	dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, ID,
			ADDR_PRO_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);

	if (dxl_comm_result != COMM_SUCCESS)
	{
		printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
	}
	else if (dxl_error != 0)
	{
		printf("%s\n", packetHandler->getRxPacketError(dxl_error));
	}

}
/*
 float DeltaRobotClass::ReadAngle(int ID)
 {
 float present_position;
 dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, ID, ADDR_PRO_PRESENT_POSITION, &present_position);
 if (dxl_comm_result != COMM_SUCCESS)
 {
 printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
 }
 //else if (dxl_error != 0)
 //{
 //	printf("%s\n", packetHandler->getRxPacketError(dxl_error));
 //}
 return (MAP(present_position, MIN_MOTOR_ANGLE, MAX_MOTOR_ANGLE, 0.0, 360.0));
 }
 */
void RasheedDeltaRobot::DriveAngle(int ID, float position)
{
	position = (int) MAP(position, 0, 360, MIN_MOTOR_ANGLE, MAX_MOTOR_ANGLE);
	dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, ID,
			ADDR_PRO_GOAL_POSITION, position, &dxl_error);
}

void RasheedDeltaRobot::SyncDriveAngle(float Deg123[])
{
	uint8_t param_goal_position1[4];
	uint8_t param_goal_position2[4];
	uint8_t param_goal_position3[4];

	int dxl_goal_position1;
	int dxl_goal_position2;
	int dxl_goal_position3;

	dxl_goal_position1 = (int) MAP(Deg123[0], 0, 360, MIN_MOTOR_ANGLE,
			MAX_MOTOR_ANGLE);
	dxl_goal_position2 = (int) MAP(Deg123[1], 0, 360, MIN_MOTOR_ANGLE,
			MAX_MOTOR_ANGLE);
	dxl_goal_position3 = (int) MAP(Deg123[2], 0, 360, MIN_MOTOR_ANGLE,
			MAX_MOTOR_ANGLE);

	// Allocate goal position value into byte array
	param_goal_position1[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_position1));
	param_goal_position1[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_position1));
	param_goal_position1[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_position1));
	param_goal_position1[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_position1));

	param_goal_position2[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_position2));
	param_goal_position2[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_position2));
	param_goal_position2[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_position2));
	param_goal_position2[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_position2));

	param_goal_position3[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_position3));
	param_goal_position3[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_position3));
	param_goal_position3[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_position3));
	param_goal_position3[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_position3));

	// Add Dynamixel#1 goal position value to the Syncwrite storage
	dxl_addparam_result = groupSyncWrite.addParam(DXL1_ID,
			param_goal_position1);
	if (dxl_addparam_result != true)
	{
		fprintf(stderr, "[ID:%03d] groupSyncWrite addparam failed", DXL1_ID);
	}

	// Add Dynamixel#2 goal position value to the Syncwrite parameter storage
	dxl_addparam_result = groupSyncWrite.addParam(DXL2_ID,
			param_goal_position2);
	if (dxl_addparam_result != true)
	{
		fprintf(stderr, "[ID:%03d] groupSyncWrite addparam failed", DXL2_ID);
	}

	// Add Dynamixel#3 goal position value to the Syncwrite parameter storage
	dxl_addparam_result = groupSyncWrite.addParam(DXL3_ID,
			param_goal_position3);
	if (dxl_addparam_result != true)
	{
		fprintf(stderr, "[ID:%03d] groupSyncWrite addparam failed", DXL3_ID);
	}

	// Syncwrite goal position
	dxl_comm_result = groupSyncWrite.txPacket();
	if (dxl_comm_result != COMM_SUCCESS)
		printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));

	// Clear syncwrite parameter storage
	groupSyncWrite.clearParam();

	//IsAllStop();
}

void RasheedDeltaRobot::SyncReadAngle(float allAngle[])
{
	int32_t dxl1_present_position = 0;
	int32_t dxl2_present_position = 0;
	int32_t dxl3_present_position = 0;

	// Syncread present position
	dxl_comm_result = groupSyncRead.txRxPacket();
	if (dxl_comm_result != COMM_SUCCESS)
	{
		printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
	}
	else if (groupSyncRead.getError(DXL1_ID, &dxl_error))
	{
		printf("[ID:%03d] %s\n", DXL1_ID,
				packetHandler->getRxPacketError(dxl_error));
	}
	else if (groupSyncRead.getError(DXL2_ID, &dxl_error))
	{
		printf("[ID:%03d] %s\n", DXL2_ID,
				packetHandler->getRxPacketError(dxl_error));
	}
	else if (groupSyncRead.getError(DXL3_ID, &dxl_error))
	{
		printf("[ID:%03d] %s\n", DXL3_ID,
				packetHandler->getRxPacketError(dxl_error));
	}

	// Check if groupsyncread data of Dynamixel#1 is available
	dxl_getdata_result = groupSyncRead.isAvailable(DXL1_ID,
			ADDR_PRO_PRESENT_POSITION, LEN_PRO_PRESENT_POSITION);
	if (dxl_getdata_result != true)
	{
		fprintf(stderr, "[ID:%03d] groupSyncRead getdata failed", DXL1_ID);

	}

	// Check if groupsyncread data of Dynamixel#2 is available
	dxl_getdata_result = groupSyncRead.isAvailable(DXL2_ID,
			ADDR_PRO_PRESENT_POSITION, LEN_PRO_PRESENT_POSITION);
	if (dxl_getdata_result != true)
	{
		fprintf(stderr, "[ID:%03d] groupSyncRead getdata failed", DXL2_ID);

	}
	// Check if groupsyncread data of Dynamixel#3 is available
	dxl_getdata_result = groupSyncRead.isAvailable(DXL3_ID,
			ADDR_PRO_PRESENT_POSITION, LEN_PRO_PRESENT_POSITION);
	if (dxl_getdata_result != true)
	{
		fprintf(stderr, "[ID:%03d] groupSyncRead getdata failed", DXL3_ID);
	}

	// Get Dynamixel#1 present position value
	dxl1_present_position = groupSyncRead.getData(DXL1_ID,
			ADDR_PRO_PRESENT_POSITION, LEN_PRO_PRESENT_POSITION);

	// Get Dynamixel#2 present position value
	dxl2_present_position = groupSyncRead.getData(DXL2_ID,
			ADDR_PRO_PRESENT_POSITION, LEN_PRO_PRESENT_POSITION);

	// Get Dynamixel#3 present position value
	dxl3_present_position = groupSyncRead.getData(DXL3_ID,
			ADDR_PRO_PRESENT_POSITION, LEN_PRO_PRESENT_POSITION);

	allAngle[0] = (float) MAP(dxl1_present_position, MIN_MOTOR_ANGLE,
			MAX_MOTOR_ANGLE, 0.0, 360.0);
	allAngle[1] = (float) MAP(dxl2_present_position, MIN_MOTOR_ANGLE,
			MAX_MOTOR_ANGLE, 0.0, 360.0);
	allAngle[2] = (float) MAP(dxl3_present_position, MIN_MOTOR_ANGLE,
			MAX_MOTOR_ANGLE, 0.0, 360.0);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// ROBOT MOVEMENT FUNCTIONS //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RasheedDeltaRobot::RobotInit(int mode)
{

	// Add parameter storage for Dynamixel#1 present position value
	dxl_addparam_result = groupSyncRead.addParam(DXL1_ID);
	if (dxl_addparam_result != true)
	{
		fprintf(stderr, "[ID:%03d] groupSyncRead addparam failed", DXL1_ID);
	}
	// Add parameter storage for Dynamixel#2 present position value
	dxl_addparam_result = groupSyncRead.addParam(DXL2_ID);
	if (dxl_addparam_result != true)
	{
		fprintf(stderr, "[ID:%03d] groupSyncRead addparam failed", DXL2_ID);
	}
	// Add parameter storage for Dynamixel#3 present position value
	dxl_addparam_result = groupSyncRead.addParam(DXL3_ID);
	if (dxl_addparam_result != true)
	{
		fprintf(stderr, "[ID:%03d] groupSyncRead addparam failed", DXL3_ID);
	}

	M1_model_number = GetModelNumber(DXL1_ID);
	M2_model_number = GetModelNumber(DXL2_ID);
	M3_model_number = GetModelNumber(DXL3_ID);

	printf("RobotInit: Settings operating mode\n");
	M1_operating_mode = mode;
	M2_operating_mode = mode;
	M3_operating_mode = mode;
	SetOperatingMode(DXL1_ID, M1_operating_mode);
	SetOperatingMode(DXL2_ID, M2_operating_mode);
	SetOperatingMode(DXL3_ID, M3_operating_mode);

	printf("RobotInit: Settings driving mode\n");
	SetDrivingMode(DXL1_ID, TIME_BASED);
	SetDrivingMode(DXL2_ID, TIME_BASED);
	SetDrivingMode(DXL3_ID, TIME_BASED);

	printf("RobotInit: Settings the motors (PID, FF)\n");
	switch (mode)
	{
	case 3:
		//Set same PID for every one
		printf("RobotInit: Setting mode 3\n");
		SetPID(DXL1_ID, PID_MODE3_P, PID_MODE3_I, PID_MODE3_D);
		SetPID(DXL2_ID, PID_MODE3_P, PID_MODE3_I, PID_MODE3_D);
		SetPID(DXL3_ID, PID_MODE3_P, PID_MODE3_I, PID_MODE3_D);

		SetFFGain(DXL1_ID, FFGAIN_MODE3_1st, FFGAIN_MODE3_2nd);
		SetFFGain(DXL2_ID, FFGAIN_MODE3_1st, FFGAIN_MODE3_2nd);
		SetFFGain(DXL3_ID, FFGAIN_MODE3_1st, FFGAIN_MODE3_2nd);
		break;
	case 5:
		// Operating mode 5 is still underdeveloped, it would be used for Dynamics Control of the robot
		printf("RobotInit: Setting mode 5\n");
		//SetPID(DXL1_ID, M1_PID_MODE5_P, M1_PID_MODE5_I, M1_PID_MODE5_D);
		//SetPID(DXL2_ID, M2_PID_MODE5_P, M2_PID_MODE5_I, M2_PID_MODE5_D);
		//SetPID(DXL3_ID, M3_PID_MODE5_P, M3_PID_MODE5_I, M3_PID_MODE5_D);

		//SetFFGain(DXL1_ID, FFGAIN_MODE5_1st, FFGAIN_MODE5_2nd);
		//SetFFGain(DXL2_ID, FFGAIN_MODE5_1st, FFGAIN_MODE5_2nd);
		//SetFFGain(DXL3_ID, FFGAIN_MODE5_1st, FFGAIN_MODE5_2nd);
		break;
	default:
		printf("RobotInit: Mode unknown recognized, PID are not set.\n");
	}
	//Gripper initialization
	SetPID(DXL4_ID, GRIPPER_PID_P, GRIPPER_PID_I, GRIPPER_PID_D);
	SetCurrentLimit(DXL4_ID, GRIPPER_CURRENT_LIMIT);
	SetGoalCurrent(DXL4_ID, GRIPPER_GOAL_CURRENT);

	SetTimeProfile(DXL1_ID, DEFAULT_TIMEACC, DEFAULT_TIMESPAN);
	SetTimeProfile(DXL2_ID, DEFAULT_TIMEACC, DEFAULT_TIMESPAN);
	SetTimeProfile(DXL3_ID, DEFAULT_TIMEACC, DEFAULT_TIMESPAN);
	SetTimeProfile(DXL4_ID, GRIPPER_Ta, GRIPPER_Tf);

	printf("The Delta Robot MK2 is ready to operate...\n");
}
void RasheedDeltaRobot::RobotTorqueON()
{
	TorqueON(DXL1_ID);
	TorqueON(DXL2_ID);
	TorqueON(DXL3_ID);
	printf("RobotTorqueON: TORQUE ON\n");

}
void RasheedDeltaRobot::RobotTorqueOFF()
{

	TorqueOFF(DXL1_ID);
	TorqueOFF(DXL2_ID);
	TorqueOFF(DXL3_ID);
	printf("RobotTorqueOFF: TORQUE OFF\n");

}

void RasheedDeltaRobot::GripperTorqueON()
{
	TorqueON(DXL4_ID);
	printf("GripperTorqueON: TORQUE ON\n");
}

void RasheedDeltaRobot::GripperTorqueOFF()
{
	TorqueOFF(DXL4_ID);
	printf("GripperTorqueOFF: TORQUE OFF\n");
}

void RasheedDeltaRobot::GripperOpen()
{
	dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, DXL4_ID,
			ADDR_PRO_GOAL_POSITION, GRIPPER_OPEN, &dxl_error);
	printf("GripperOpen: Open\n");
}

void RasheedDeltaRobot::GripperClose()
{
	dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, DXL4_ID,
			ADDR_PRO_GOAL_POSITION, GRIPPER_CLOSE, &dxl_error);
	printf("GripperClose: Close\n");
}

void RasheedDeltaRobot::GripperCheck()
{
	GripperOpen();
	usleep(1000000);
	GripperClose();
	usleep(1000000);
	GripperOpen();
	usleep(1000000);
	printf("GripperCheck: Release an object\n");
}

int RasheedDeltaRobot::XYZOutRange(float X, float Y, float Z)
{
	int WarningFlag = 0;
	float workingR;
	float maxZ;

	workingR = sqrt(pow(X, 2) + pow(Y, 2));

	maxZ = 0.75 * workingR - 1225.0;

	if ((workingR > maxR) || (Z < maxStoke) || (Z < maxZ) || (Z > minStoke))
	{
		WarningFlag = 1;
		printf("XYZOutRange: input value is invalid, try change X, Y or Z \n");
	}
	else
	{
		WarningFlag = 0;
	}

	return WarningFlag;
}

void RasheedDeltaRobot::IsAllStop()
{
	int MovingFlag = 1;
	int delayMicrosec;
	delayMicrosec = (DEFAULT_TIMESPAN / 2) * 1000; //Find a proper delay value from total time of movement
	usleep(delayMicrosec); // a bit of delay for acceleration period, if not the while loop down there will skip

	while (MovingFlag)
	{
		//printf("IsMoving1: %d \n", IsMoving(DXL1_ID));
		//printf("IsMoving2: %d \n", IsMoving(DXL2_ID));
		//printf("IsMoving3: %d \n", IsMoving(DXL3_ID));
		if ((IsMoving(DXL1_ID) == 0) && (IsMoving(DXL2_ID) == 0)
				&& (IsMoving(DXL3_ID) == 0))
		{
			MovingFlag = 0;
			printf("IsAllStop: All servos are stop \n");
		}
		usleep((DEFAULT_TIMESPAN / 20) * 1000); // a bit of delay to make sure the servo is stop or moving slowly

	}

}

void RasheedDeltaRobot::GotoPoint(float X, float Y, float Z)
{
	int WARN = 0;
	float wantedXYZ[3] =
	{ X, Y, Z };
	float GoalAngle[3];

	WARN = XYZOutRange(X, Y, Z);
	if (WARN == 0)
	{
		INV(wantedXYZ, GoalAngle);

		// 180 is the offset when attach the bracket to the servo
		// when robot rotate to 0deg, the servo should turns to 180deg
		GoalAngle[0] = GoalAngle[0] + 180;
		GoalAngle[1] = GoalAngle[1] + 180;
		GoalAngle[2] = GoalAngle[2] + 180;

		printf("GoalAngle[0]: %f \n", GoalAngle[0]);
		printf("GoalAngle[1]: %f \n", GoalAngle[1]);
		printf("GoalAngle[2]: %f \n", GoalAngle[2]);

		// Check whether the feeding value is Nan or not, if Nan then don't drive the servo
		if ((isnan(GoalAngle[0]) != 1) && (isnan(GoalAngle[1]) != 1)
				&& (isnan(GoalAngle[2]) != 1))
		{
			SyncDriveAngle(GoalAngle);
			// CheckTo make sure the robot is stop before driving to the next goal point
			IsAllStop();
			printf("GotoPoint: Success go to point (%f, %f, %f) \n", X, Y, Z);
		}

	}
	else
	{
		printf("GotoPoint: error input XYZ is out of working range! \n");
	}
}

void RasheedDeltaRobot::GetXYZ(float currentXYZ[])
{
	float servoAngle[3];
	float robotAngle[3];
	SyncReadAngle(servoAngle);

	// -180 is an offset of servo bracket
	// means that if SyncReadAngle read data from servo then we need to -180 to make it an angle for robot joint
	robotAngle[0] = servoAngle[0] - 180;
	robotAngle[1] = servoAngle[1] - 180;
	robotAngle[2] = servoAngle[2] - 180;

	FWD(robotAngle, currentXYZ);

	//printf("currentX %f \n", currentXYZ[0]);
	//printf("currentY %f \n", currentXYZ[1]);
	//printf("currentZ %f \n", currentXYZ[2]);
	//printf("-----------------------------\n");

}

void RasheedDeltaRobot::GetRobotAngle(float robotAngle[])
{
	float servoAngle[3];
	SyncReadAngle(servoAngle);

	// -180 is an offset of servo bracket
	// means that if SyncReadAngle read data from servo then we need to -180 to make it an angle for robot joint
	robotAngle[0] = servoAngle[0] - 180;
	robotAngle[1] = servoAngle[1] - 180;
	robotAngle[2] = servoAngle[2] - 180;

	//printf("joint1 %f \n", robotAngle[0]);
	//printf("joint2 %f \n", robotAngle[1]);
	//printf("joint3 %f \n", robotAngle[2]);
	//printf("-----------------------------\n");

}

void RasheedDeltaRobot::GoHome()
{
	float ServoHomePos[3] =
	{ 180.0, 180.0, 180.0 };

	SyncDriveAngle(ServoHomePos);
	printf("GoHome: Moving back home... \n");
	IsAllStop();
	printf("GoHome: Robot is returned to home position \n");
}

void RasheedDeltaRobot::KinematicsCheck()
{
	float RobotAngle[3];
	float RobotAngleWithOffset[3];
	float XYZfromFWD[3];
	float AngfromINV[3];
	float ConfirmXYZ[3];

	GetRobotAngle(RobotAngle);
	printf("RobotAngle J1: %f \n", RobotAngle[0]);
	printf("RobotAngle J2: %f \n", RobotAngle[1]);
	printf("RobotAngle J3: %f \n", RobotAngle[2]);
	printf("-----------------------------------\n");

	RobotAngleWithOffset[0] = RobotAngle[0] + 3.8427;
	RobotAngleWithOffset[1] = RobotAngle[1] + 3.8427;
	RobotAngleWithOffset[2] = RobotAngle[2] + 3.8427;

	printf("RobotAngleWithOffset J1: %f \n", (RobotAngle[0] + 3.8427));
	printf("RobotAngleWithOffset J2: %f \n", (RobotAngle[1] + 3.8427));
	printf("RobotAngleWithOffset J3: %f \n", (RobotAngle[2] + 3.8427));
	printf("-----------------------------------\n");

	FWD(RobotAngle, XYZfromFWD);
	printf("X_FWD: %f \n", XYZfromFWD[0]);
	printf("Y_FWD: %f \n", XYZfromFWD[1]);
	printf("Z_FWD: %f \n", XYZfromFWD[2]);
	printf("-----------------------------------\n");

	INV(XYZfromFWD, AngfromINV);
	printf("Ang1_INV: %f \n", AngfromINV[0]);
	printf("Ang2_INV: %f \n", AngfromINV[1]);
	printf("Ang2_INV: %f \n", AngfromINV[2]);
	printf("-----------------------------------\n");

	FWD(AngfromINV, ConfirmXYZ);
	printf("X_FWD_confirm: %f \n", ConfirmXYZ[0]);
	printf("Y_FWD_confirm: %f \n", ConfirmXYZ[1]);
	printf("Z_FWD_confirm: %f \n", ConfirmXYZ[2]);
	printf("-----------------------------------\n");
	printf("-----------------------------------\n");
	printf("-----------------------------------\n");

}

#endif

