#include "motorMX430.h"

//Specify to initialize members in the member initialiser list:
MotorXM430::MotorXM430()
{
}

bool MotorXM430::init(int ID, int operating_mode, int current_limit, int goal_current, const char* portName, int baudRate)
{
	portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);
	packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

	//Communication initialisation
	dxl_comm_result = COMM_TX_FAIL;
	dxl_error = 0;
	if (portHandler->openPort())
	{
	}
	else
	{
		printf("Motor %d: Failed to open the port!\n", m_ID);
		return false;
	}

	if (portHandler->setBaudRate(BAUDRATE))
	{
	}
	else
	{
		printf("Motor %d: Failed to change the baudrate!\n", m_ID);
		return false;
	}

	//Motor initialisation:
	m_ID = ID;
	m_model_number = GetModelNumber();

	m_mode = operating_mode;
	SetOperatingMode(m_mode);

	m_current_limit = current_limit;
	SetCurrentLimit();

	m_goal_current = goal_current;
	SetGoalCurrent(goal_current);

	m_present_position = ReadAngle();

	printf("UPDATE: drivingmode setting: TIME_BASED MODE\n");
	SetDrivingMode(TIME_BASED);
	m_drivingmode = PrintDrivingMode();

	printf("Motor %d initialized\n", m_ID);

	return true;
}

int MotorXM430::GetID()
{
	return m_ID;
}

//Updated 2019/02/08
void MotorXM430::SetDrivingMode(uint8_t type)
{
	dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, m_ID,
			ADDR_PRO_DRIVE_MODE, type);
}
uint8_t MotorXM430::PrintDrivingMode()
{
	uint8_t type;
	dxl_comm_result = packetHandler->read1ByteTxRx(portHandler, m_ID,
			ADDR_PRO_DRIVE_MODE, &type);
	printf("Motor %d is on driving mode: %d\n", m_ID, type);
	return (type);
}

void MotorXM430::SetTimeProfile(uint32_t Ta, uint32_t Tf)
{
	dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, m_ID,
			ADDR_PRO_PROFILE_ACCELERATION_TIME, Ta);
	dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, m_ID,
			ADDR_PRO_PROFILE_TIME_SPAN, Tf);
}

void MotorXM430::PrintTimeProfile()
{
	uint32_t Ti, Tf;
	dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, m_ID,
			ADDR_PRO_PROFILE_ACCELERATION_TIME, &Ti);
	dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, m_ID,
			ADDR_PRO_PROFILE_TIME_SPAN, &Tf);
	printf("Motor %d, current acceleration time: %d / timespan: %d\n", m_ID, Ti,
			Tf);
}

//End update
uint16_t MotorXM430::GetModelNumber()
{
	uint16_t modelNumber = 0;
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, m_ID,
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

uint8_t MotorXM430::IsMoving()
{
	uint8_t moving;
	dxl_comm_result = packetHandler->read1ByteTxRx(portHandler, m_ID,
			ADDR_PRO_MOVING, &moving);
	return (moving);
}

void MotorXM430::MovingStatus()
{
	uint8_t movingstatus = 0;
	dxl_comm_result = packetHandler->read1ByteTxRx(portHandler, m_ID,
			ADDR_PRO_MOVING_STATUS, &movingstatus);
	if (movingstatus > 48)
	{
		printf("Motor %d is in Trapezodal Profile\n", m_ID);
	}
	else if ((movingstatus < 35) && (movingstatus > 20))
	{
		printf("Motor %d is in Triangular Profile\n", m_ID);
	}
	else if (movingstatus > 3)
	{
		printf("Motor %d is in Rectangular Profile\n", m_ID);
	}
	else if (movingstatus >= 0)
	{
		printf("Motor %d is in Step Mode (No Profile)", m_ID);
	}
	else
	{
		printf("Motor %d UNKNOWN Profile\n", m_ID);
	}
}

int16_t MotorXM430::ReadCurrent()
{
	uint16_t current = 0;
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_PRESENT_CURRENT, &current);
	int16_t currentS = current;
	//printf("Implicit conversion: %d\n",currentS);
	return (currentS);
}

float MotorXM430::MAP(uint32_t angle, long in_min, long in_max, long out_min,
		long out_max)
{
	return (((float) angle - in_min) * (out_max - out_min) / (in_max - in_min)
			+ out_min);
}

void MotorXM430::TorqueON()
{
	dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, m_ID,
			ADDR_PRO_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);

	if (dxl_comm_result != COMM_SUCCESS)
	{
		printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
	}
	else if (dxl_error != 0)
	{
		printf("%s\n", packetHandler->getRxPacketError(dxl_error));
	}
	else
	{
		//printf("Motor %d: TORQUE ENABLED\n", m_ID);
	}
}

void MotorXM430::TorqueOFF()
{
	dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, m_ID,
			ADDR_PRO_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);

	if (dxl_comm_result != COMM_SUCCESS)
	{
		printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
	}
	else if (dxl_error != 0)
	{
		printf("%s\n", packetHandler->getRxPacketError(dxl_error));
	}
	else
	{
		//printf("Motor %d: TORQUE DISABLE\n", m_ID);
	}
}

float MotorXM430::ReadAngle()
{
	dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, m_ID,
			ADDR_PRO_PRESENT_POSITION, &m_present_position, &dxl_error);
	if (dxl_comm_result != COMM_SUCCESS)
	{
		printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
	}
	else if (dxl_error != 0)
	{
		printf("%s\n", packetHandler->getRxPacketError(dxl_error));
	}
	return (MAP(m_present_position, MIN_MOTOR_ANGLE, MAX_MOTOR_ANGLE, 0.0,
			360.0));
}

void MotorXM430::Goto(float position)
{
	position = (int) MAP(position, 0, 360, MIN_MOTOR_ANGLE, MAX_MOTOR_ANGLE);
	dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, m_ID,
			ADDR_PRO_GOAL_POSITION, position, &dxl_error);
}

void MotorXM430::SetOperatingMode(uint8_t mode)
{
	TorqueOFF();
	if ((mode == 0) || (mode == 1) || (mode == 3) || (mode == 4) || (mode == 5)
			|| (mode == 16))
	{
		dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, m_ID,
				ADDR_PRO_OPERATING_MODE, mode, &dxl_error);
		if (dxl_comm_result != COMM_SUCCESS)
		{
			printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
		}
		else if (dxl_error != 0)
		{
			printf("%s\n", packetHandler->getRxPacketError(dxl_error));
		}
		else
		{
			m_mode = mode;
		}
	}
	else
	{
		printf("Invalid Control Mode (Availiable mode: 0, 1, 3, 4, 5, 16)\n");
	}

}

void MotorXM430::PrintOperatingMode()
{
	uint8_t mode = 3;
	dxl_comm_result = packetHandler->read1ByteTxRx(portHandler, m_ID,
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

void MotorXM430::SetPID(uint16_t P_gain, uint16_t I_gain, uint16_t D_gain)
{
	dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_POSITION_P_GAIN, P_gain, &dxl_error);
	dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_POSITION_I_GAIN, I_gain, &dxl_error);
	dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_POSITION_D_GAIN, D_gain, &dxl_error);
	// check if PID are set:
	PrintPID();
}
void MotorXM430::PrintPID()
{
	uint16_t P, I, D;
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_POSITION_P_GAIN, &P, &dxl_error);
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_POSITION_I_GAIN, &I, &dxl_error);
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_POSITION_D_GAIN, &D, &dxl_error);
	printf("Motor %d: 	PID values : P: %d / I: %d / D: %d\n", m_ID, P, I, D);
}

void MotorXM430::SetFFGain(uint16_t FF1Gain, uint16_t FF2Gain)
{
	dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_FEEDFORWARD_1st_GAIN, FF1Gain);
	dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_FEEDFORWARD_2nd_GAIN, FF2Gain);
	// check if FFgain are set:
	PrintFFGain();
}

void MotorXM430::PrintFFGain()
{
	uint16_t FF1Gain, FF2Gain;
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_FEEDFORWARD_1st_GAIN, &FF1Gain);
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_FEEDFORWARD_2nd_GAIN, &FF2Gain);
	printf("Motor %d, Feed forward Gain: FF1: %d / FF2: %d\n", m_ID, FF1Gain,
			FF2Gain);
}

void MotorXM430::SetProfile(uint32_t Velocity, uint32_t Acceleration)
{
	//Set the limits:
	dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, m_ID,
			ADDR_PRO_VELOCITY_LIMIT, VELOCITY_LIMIT);
	dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, m_ID,
			ADDR_PRO_ACCELERATION_LIMIT, ACCELERATION_LIMIT);
	if (Velocity <= VELOCITY_LIMIT)
	{
		dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, m_ID,
				ADDR_PRO_PROFILE_VELOCITY, Velocity);
	}
	else
	{
		printf("Velocity out of range (limit=%d)\n", VELOCITY_LIMIT);
	}
	if (Acceleration <= ACCELERATION_LIMIT)
	{
		dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, m_ID,
				ADDR_PRO_PROFILE_ACCELERATION, Acceleration);
	}
	else
	{
		printf("Acceleration out of range (limit=%d)\n", ACCELERATION_LIMIT);
	}
}

void MotorXM430::PrintProfile()
{
	uint32_t acceleration = 0;
	uint32_t velocity = 0;
	dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, m_ID,
			ADDR_PRO_PROFILE_VELOCITY, &velocity);
	dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, m_ID,
			ADDR_PRO_PROFILE_ACCELERATION, &acceleration);
	printf("Motor %d:	Acceleration Profile: %d and Velocity Profile: %d\n",
			m_ID, acceleration, velocity);
}

void MotorXM430::SetGoalCurrent(uint16_t GoalCurrent)
{
	if ((GoalCurrent >= -m_current_limit) || (GoalCurrent <= m_current_limit))
	{
		dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, m_ID,
				ADDR_PRO_GOAL_CURRENT, GoalCurrent);
	}
	else
	{
		printf("Goal current out of range (limit=%d)\n", m_current_limit);
	}
}

void MotorXM430::PrintGoalCurrent()
{
	uint16_t goalCurrent = 0;
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_GOAL_CURRENT, &goalCurrent);
	printf("Motor %d: Goal Current: %d mA\n", m_ID, goalCurrent);
}

void MotorXM430::SetCurrentLimit()
{
	dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_CURRENT_LIMIT, m_current_limit);
}

void MotorXM430::PrintCurrentLimit()
{
	float SetUnit = 1;
	uint16_t current_limit = CURRENT_LIMIT;
	float current_limit_amp = 0;
	float UnitMX06 = 3.96;
	float UnitMX540 = 2.69;
	float UnitMX430 = 2.69;
	switch (m_model_number)
	{
	case 321:
		SetUnit = UnitMX06;
		break;
	case 1020:
		SetUnit = UnitMX430;
		break;
	case 1120:
		SetUnit = UnitMX540;
		break;
	default:
		printf("Motor %d, model not supported.\n", m_ID);
	}
	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, m_ID,
			ADDR_PRO_CURRENT_LIMIT, &current_limit);
	current_limit_amp = (float) current_limit * (SetUnit / 1000.0);
	printf("Motor %d, Current limit: %d (input motor), %f (amp)\n", m_ID,
			current_limit, current_limit_amp);
}
