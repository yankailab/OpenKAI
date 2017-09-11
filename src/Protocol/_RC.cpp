#include "../Protocol/_RC.h"

namespace kai
{
_RC::_RC()
{
	m_pIO = NULL;
}

_RC::~_RC()
{
	close();
}

bool _RC::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	//TODO: Read config


	return true;
}

bool _RC::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void _RC::close()
{
	if (m_pIO)
	{
		m_pIO->close();
		delete m_pIO;
	}
	printf("Serial port closed.\n");
}

bool _RC::readMessages()
{
	uint8_t	inByte;
	int		byteRead;

	while ((byteRead = m_pIO->read(&inByte,1)) > 0)
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

void _RC::rc_overide(int numChannel, int* pChannels)
{
	NULL_(m_pIO);
	IF_(!m_pIO->isOpen());

	int len;

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
	m_pIO->write(m_pBuf, len);

}

void _RC::controlMode(int mode)
{
	NULL_(m_pIO);
	IF_(!m_pIO->isOpen());

	m_pBuf[0] = 0xFE;//Mavlink begin
	m_pBuf[1] = 1;
	m_pBuf[2] = 1;
	m_pBuf[3] = (unsigned char)mode;

	m_pIO->write(m_pBuf, 4);
}

bool _RC::start(void)
{
	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _RC::update(void)
{
	while (m_bThreadON)
	{
		IF_CONT(!m_pIO);

		if(!m_pIO->isOpen())
		{
			if(!m_pIO->open())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
			m_recvMsg.m_cmd = 0;
		}


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
