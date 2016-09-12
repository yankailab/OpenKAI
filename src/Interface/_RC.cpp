#include "_RC.h"

namespace kai
{
_RC::_RC()
{
	_ThreadBase();

	m_bConnected = false;
	m_sportName = "";
	m_pSerialPort = NULL;
	m_baudRate = 115200;
}

_RC::~_RC()
{
	if (m_pSerialPort)
	{
		delete m_pSerialPort;
	}
}

bool _RC::setup(JSON* pJson, string serialName)
{
	if(!pJson)return false;

	CHECK_ERROR(pJson->var("SERIALPORT_"+serialName+"_NAME", &m_sportName));
	CHECK_ERROR(pJson->var("SERIALPORT_"+serialName+"_BAUDRATE", &m_baudRate));

	this->setTargetFPS(1000);

	return true;
}

bool _RC::open(void)
{
	//Start Serial Port
	m_pSerialPort = new SerialPort();
	if (m_pSerialPort->Open((char*)m_sportName.c_str()) != true)
	{
		m_bConnected = false;
		return false;
	}

	if (!m_pSerialPort->Setup(m_baudRate, 8, 1, false, false))
	{
		printf("failure, could not configure port.\n");
		return false;
	}

	m_recvMsg.m_cmd = 0;
	m_bConnected = true;

	return true;
}

void _RC::close()
{
//	disable_offboard_control();

	if (m_pSerialPort)
	{
		delete m_pSerialPort;
		m_bConnected = false;
	}
	printf("Serial port closed.\n");
}

// ------------------------------------------------------------------------------
//   Read Messages
// ------------------------------------------------------------------------------
bool _RC::readMessages()
{
	unsigned char	inByte;
	int		byteRead;

	while ((byteRead = m_pSerialPort->Read((char*)&inByte,1)) > 0)
	{
		if (m_recvMsg.m_cmd != 0)
		{
			m_recvMsg.m_pBuf[m_recvMsg.m_iByte] = inByte;
			m_recvMsg.m_iByte++;

			if (m_recvMsg.m_iByte == 2)	//Payload length
			{
				m_recvMsg.m_payloadLen = inByte;
			}
			else if (m_recvMsg.m_iByte == m_recvMsg.m_payloadLen + MAVLINK_HEADDER_LEN + 1)
			{
				//decode the command
//				hostCommand();
				m_recvMsg.m_cmd = 0;

				return true; //Execute one command at a time
			}
		}
		else if (inByte == MAVLINK_BEGIN)//(inByte == VL_CMD_BEGIN || inByte == MAVLINK_BEGIN)
		{
			m_recvMsg.m_cmd = inByte;
			m_recvMsg.m_pBuf[0] = inByte;
			m_recvMsg.m_iByte = 1;
			m_recvMsg.m_payloadLen = 0;
		}
	}

	return false;

}



// ------------------------------------------------------------------------------
//   Write Setpoint Message
// ------------------------------------------------------------------------------
void _RC::rc_overide(int numChannel, int* pChannels)
{
	if (!m_bConnected)return;

	int len;
	int pwm;

	m_pBuf[0] = 0xFE;//Mavlink begin
	m_pBuf[1] = 1 + numChannel * 2;	//Payload Length
	m_pBuf[2] = 0;
	m_pBuf[3] = numChannel;

	for (int i = 0; i < numChannel; i++)
	{
		m_pBuf[4 + i * 2] = (unsigned char)(pChannels[i] & 0xFF);
		m_pBuf[4 + i * 2 + 1] = (unsigned char)(pChannels[i] >> 8);
	}

	len = 4 + numChannel * 2;

	m_pSerialPort->Write((char*)m_pBuf, len);

}

void _RC::controlMode(int mode)
{
	if (!m_bConnected)return;

	m_pBuf[0] = 0xFE;//Mavlink begin
	m_pBuf[1] = 1;
	m_pBuf[2] = 1;
	m_pBuf[3] = (unsigned char)mode;

	m_pSerialPort->Write((char*)m_pBuf, 4);
}



bool _RC::start(void)
{
	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode << " in VehicleInterface::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO) << "VehicleInterface.start()";

	return true;
}

void _RC::update(void)
{

	while (m_bThreadON)
	{
		this->autoFPSfrom();

/*
		if (g_pVehicle->open((char*)g_serialPort.c_str()) != true)
		{
			LOG(ERROR)<< "Cannot open serial port, Working in CV mode only";
		}
		*/

		this->autoFPSto();
	}

}


}
