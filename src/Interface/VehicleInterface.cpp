#include "VehicleInterface.h"

namespace kai
{
VehicleInterface::VehicleInterface()
{
	m_threadID = 0;
	m_bConnected = false;
	m_sportName = "";
	m_bThreadON = false;
	m_pSerialPort = NULL;
}

VehicleInterface::~VehicleInterface()
{
}

void VehicleInterface::setSerialName(string name)
{
	m_sportName = name;
}

bool VehicleInterface::open(void)
{
	m_systemID = 0; // system id
	m_autopilotID = 0; // autopilot component id
	m_companionID = 0; // companion computer component id

//	m_currentMessages.sysid = m_systemID;
//	m_currentMessages.compid = m_autopilotID;

	//Start Serial Port
	m_pSerialPort = new SerialPort();
	if (m_pSerialPort->Open((char*)m_sportName.c_str()) != true)
	{
		m_bConnected = false;
		return false;
	}

	m_recvMsg.m_cmd = 0;
	m_bConnected = true;

	return true;


}

void VehicleInterface::close()
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
bool VehicleInterface::readMessages()
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
void VehicleInterface::rc_overide(int numChannel, int* pChannels)
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

void VehicleInterface::controlMode(int mode)
{
	if (!m_bConnected)return;

	m_pBuf[0] = 0xFE;//Mavlink begin
	m_pBuf[1] = 1;
	m_pBuf[2] = 1;
	m_pBuf[3] = (unsigned char)mode;

	m_pSerialPort->Write((char*)m_pBuf, 4);
}



bool VehicleInterface::start(void)
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

void VehicleInterface::update(void)
{
	m_tSleep = TRD_INTERVAL_VI_USEC;

	while (m_bThreadON)
	{
		this->updateTime();



/*
		if (g_pVehicle->open((char*)g_serialPort.c_str()) != true)
		{
			LOG(ERROR)<< "Cannot open serial port, Working in CV mode only";
		}
		*/



		if(m_tSleep>0)
		{
			//sleepThread can be woke up by this->wakeupThread()
			this->sleepThread(0,m_tSleep);
		}
	}

}

bool VehicleInterface::complete(void)
{

	return true;
}

void VehicleInterface::stop(void)
{
	m_bThreadON = false;
	this->wakeupThread();
	pthread_join(m_threadID, NULL);

	LOG(INFO) << "VehicleInterface.stop()";
}

void VehicleInterface::waitForComplete(void)
{
	pthread_join(m_threadID, NULL);
}

}
