#include "_Canbus.h"

namespace kai
{
_Canbus::_Canbus()
{
	m_pSerialPort = NULL;
}

_Canbus::~_Canbus()
{
	close();
}

bool _Canbus::init(void* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	Kiss* pCC = pK->o("input");
	CHECK_F(pCC->empty());

	m_pSerialPort = new SerialPort();
	CHECK_F(!m_pSerialPort->init(pCC));

	return true;
}

bool _Canbus::link(void)
{
	CHECK_F(!this->_ThreadBase::link());
	return true;
}

void _Canbus::close()
{
	if (m_pSerialPort)
	{
		m_pSerialPort->close();
		delete m_pSerialPort;
	}

	LOG_I("Closed");
}

bool _Canbus::start(void)
{
	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Canbus::update(void)
{
	while (m_bThreadON)
	{
		if(!m_pSerialPort->isOpen())
		{
			if(!m_pSerialPort->open())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		//Regular update loop
		this->autoFPSfrom();

		//Handling incoming messages
		recv();

		this->autoFPSto();
	}

}

bool _Canbus::recv()
{
	unsigned char inByte;
	int byteRead;

	while ((byteRead = m_pSerialPort->read(&inByte, 1)) > 0)
	{
//		printf("%s",&inByte);
	}
//	LOG_I("Received");
	return true;




	while ((byteRead = m_pSerialPort->read(&inByte, 1)) > 0)
	{
		if (m_recvMsg.m_cmd != 0)
		{
			m_recvMsg.m_pBuf[m_recvMsg.m_iByte] = inByte;
			m_recvMsg.m_iByte++;

			if (m_recvMsg.m_iByte == 2)	//Payload length
			{
				m_recvMsg.m_payloadLen = inByte;
			}
			else if (m_recvMsg.m_iByte
					== m_recvMsg.m_payloadLen + MAVLINK_HEADDER_LEN + 1)
			{
				//decode the command
//				hostCommand();
				m_recvMsg.m_cmd = 0;

				return true; //Execute one command at a time
			}
		}
		else if (inByte == MAVLINK_BEGIN)
		{
			m_recvMsg.m_cmd = inByte;
			m_recvMsg.m_pBuf[0] = inByte;
			m_recvMsg.m_iByte = 1;
			m_recvMsg.m_payloadLen = 0;
		}
	}

	return false;

}

void _Canbus::send(unsigned long addr, unsigned char len, unsigned char* pData)
{
	CHECK_(len+8 > CAN_BUF);
	CHECK_(!m_pSerialPort->isOpen());

	//Link header
	m_pBuf[0] = 0xFE; //Mavlink begin
	m_pBuf[1] = 5 + len;	//Payload Length
	m_pBuf[2] = CMD_CAN_SEND;

	//Payload from here

	//CAN addr
	m_pBuf[3] = (uint8_t)addr;
	m_pBuf[4] = (uint8_t)(addr >> 8);
	m_pBuf[5] = (uint8_t)(addr >> 16);
	m_pBuf[6] = (uint8_t)(addr >> 24);

	//CAN data len
	m_pBuf[7] = len;

	for (int i = 0; i < len; i++)
	{
		m_pBuf[i + 8] = pData[i];
	}

	//Payload to here

	m_pSerialPort->write(m_pBuf, len + 8);
}


bool _Canbus::draw(void)
{
	CHECK_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;

	string msg = *this->getName();
	if (m_pSerialPort->isOpen())
		msg += ": CONNECTED";
	else
		msg += ": Not connected";

	pWin->addMsg(&msg);

	return true;
}

}
