#include "_Canbus.h"

namespace kai
{
_Canbus::_Canbus()
{
	_ThreadBase();

	m_sportName = "";
	m_pSerialPort = NULL;
	m_baudRate = 115200;
}

_Canbus::~_Canbus()
{
	close();
}

bool _Canbus::init(Config* pConfig)
{
	CHECK_F(!this->_ThreadBase::init(pConfig));

	pConfig->m_pInst = this;

	F_ERROR_F(pConfig->v("portName", &m_sportName));
	F_ERROR_F(pConfig->v("baudrate", &m_baudRate));

	//Start Serial Port
	m_pSerialPort = new SerialPort();

	return true;
}

bool _Canbus::link(void)
{
	NULL_F(m_pConfig);

	//TODO: link variables to Automaton

	return true;
}

void _Canbus::close()
{
	if (m_pSerialPort)
	{
		m_pSerialPort->Close();
		delete m_pSerialPort;
	}
	printf("Serial port closed.\n");
}

bool _Canbus::recv()
{
	unsigned char inByte;
	int byteRead;

	while ((byteRead = m_pSerialPort->Read((char*) &inByte, 1)) > 0)
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
	CHECK_(!m_pSerialPort->IsConnected());

	m_pBuf[0] = 0xFE; //Mavlink begin
	m_pBuf[1] = 4 + 1 + len;	//Payload Length
	m_pBuf[2] = CMD_CAN_SEND;

	for (int i = 0; i < len; i++)
	{
		m_pBuf[i + 3] = pData[i];
	}

	//	CAN.sendMsgBuf(*addr, 0, len, pData);

	m_pSerialPort->Write((char*) m_pBuf, len + 3);
}

bool _Canbus::start(void)
{
	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Canbus::update(void)
{
	while (m_bThreadON)
	{
		//Establish serial connection
		if (m_sportName == "")
		{
			this->sleepThread(1, 0);
			continue;
		}

		//Try to open and setup the serial port
		if (!m_pSerialPort->IsConnected())
		{
			if (m_pSerialPort->Open((char*) m_sportName.c_str()))
			{
				LOG(INFO)<< "Serial port: "+m_sportName+" connected";
			}
			else
			{
				this->sleepThread(1, 0);
				continue;
			}

			if (!m_pSerialPort->Setup(m_baudRate, 8, 1, false, false))
			{
				LOG(INFO)<< "Serial port: "+m_sportName+" could not be configured";
				m_pSerialPort->Close();
				this->sleepThread(1, 0);
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

bool _Canbus::draw(Frame* pFrame, iVec4* pTextPos)
{
	NULL_F(pFrame);

	Mat* pMat = pFrame->getCMat();

	char strBuf[512];
	std::string strInfo;

	if (m_pSerialPort->IsConnected())
	{
		putText(*pFrame->getCMat(), "CANBUS FPS: " + i2str(getFrameRate()),
				cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX,
				0.5, Scalar(0, 255, 0), 1);
	}
	else
	{
		putText(*pFrame->getCMat(), "CANBUS Not Connected",
				cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX,
				0.5, Scalar(0, 255, 0), 1);
	}

	pTextPos->m_y += pTextPos->m_w;

	//Vehicle position
//	sprintf(strBuf, "Attitude: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
//			current_messages.attitude.roll, current_messages.attitude.pitch, current_messages.attitude.yaw);
//	PUTTEXT(pTextPos->m_x, pTextPos->m_y, strBuf);
//	pTextPos->m_y += pTextPos->m_w;

	return true;
}

}
