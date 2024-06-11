#include "_Canbus.h"

namespace kai
{

	_Canbus::_Canbus()
	{
		m_pIO = NULL;
		m_nCanData = 0;
		m_recvMsg.init();
	}

	_Canbus::~_Canbus()
	{
	}

	bool _Canbus::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
    	

		m_nCanData = 0;
		while (1)
		{
			IF_F(m_nCanData >= N_CANDATA);
			Kiss *pAddr = pK->child(m_nCanData);
			if (pAddr->empty())
				break;

			m_pCanData[m_nCanData].init();
			F_ERROR_F(pAddr->v("addr", (int *)&(m_pCanData[m_nCanData].m_addr)));
			m_nCanData++;
		}

		string n;
		n = "";
		F_ERROR_F(pK->v("_IObase", &n));
		m_pIO = (_SerialPort *)(pK->getInst(n));
		NULL_Fl(m_pIO, "_IObase not found");

		return true;
	}

	bool _Canbus::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Canbus::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_pIO)
			{
				m_pT->sleepT(SEC_2_USEC);
				continue;
			}

			if (!m_pIO->bOpen())
			{
				if (!m_pIO->open())
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			//Regular update loop
			m_pT->autoFPSfrom();

			//Handling incoming messages
			recv();

			m_pT->autoFPSto();
		}
	}

	bool _Canbus::recv()
	{
		unsigned char inByte;
		int byteRead;

		while ((byteRead = m_pIO->read(&inByte, 1)) > 0)
		{
			if (m_recvMsg.m_cmd != 0)
			{
				m_recvMsg.m_pBuf[m_recvMsg.m_iByte] = inByte;
				m_recvMsg.m_iByte++;

				if (m_recvMsg.m_iByte == 2) //Payload length
				{
					m_recvMsg.m_payloadLen = inByte;
				}
				else if (m_recvMsg.m_iByte == m_recvMsg.m_payloadLen + MAVLINK_HEADDER_LEN + 1)
				{
					//decode the command
					recvMsg();
					m_recvMsg.m_cmd = 0;

					return true; //Execute one command at a time
				}
				else if (m_recvMsg.m_iByte >= CAN_BUF)
				{
					m_recvMsg.m_cmd = 0;
					return false;
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

	void _Canbus::recvMsg(void)
	{
		uint8_t cmd = m_recvMsg.m_pBuf[2];

		if (cmd == CAN_CMD_CAN_SEND)
		{
			uint32_t *pAddr = (uint32_t *)(&m_recvMsg.m_pBuf[3]);
			uint32_t addr = *pAddr;

			for (int i = 0; i < m_nCanData; i++)
			{
				CAN_DATA *pCan = &m_pCanData[i];
				IF_CONT(pCan->m_addr != addr);

				pCan->m_len = m_recvMsg.m_pBuf[7];
				memcpy(pCan->m_pData, &m_recvMsg.m_pBuf[8], 8);

				LOG_I("Updated data for CANID:" + i2str(addr));
				return;
			}

			LOG_I("CANID:" + i2str(addr));
		}
	}

	uint8_t *_Canbus::get(unsigned long addr)
	{
		for (int i = 0; i < m_nCanData; i++)
		{
			CAN_DATA *pCan = &m_pCanData[i];
			IF_CONT(pCan->m_addr != addr);

			return pCan->m_pData;
		}

		return NULL;
	}

	void _Canbus::send(unsigned long addr, unsigned char len, unsigned char *pData)
	{
		IF_(len + 8 > CAN_BUF);
		IF_(!m_pIO->bOpen());

		unsigned char pBuf[CAN_BUF];

		//Link header
		pBuf[0] = 0xFE;	   //Mavlink begin
		pBuf[1] = 5 + len; //Payload Length
		pBuf[2] = CAN_CMD_CAN_SEND;

		//Payload from here
		//CAN addr
		pBuf[3] = (uint8_t)addr;
		pBuf[4] = (uint8_t)(addr >> 8);
		pBuf[5] = (uint8_t)(addr >> 16);
		pBuf[6] = (uint8_t)(addr >> 24);

		//CAN data len
		pBuf[7] = len;

		for (int i = 0; i < len; i++)
		{
			pBuf[i + 8] = pData[i];
		}
		//Payload to here

		m_pIO->write(pBuf, len + 8);
	}

	void _Canbus::pinOut(uint8_t pin, uint8_t output)
	{
		IF_(!m_pIO->bOpen());

		unsigned char pBuf[CAN_BUF];

		//Link header
		pBuf[0] = 0xFE; //Mavlink begin
		pBuf[1] = 2;	//Payload Length
		pBuf[2] = CAN_CMD_PIN_OUTPUT;

		//Payload from here
		//CAN addr
		pBuf[3] = pin;
		pBuf[4] = output;
		//Payload to here

		m_pIO->write(pBuf, 5);
	}

	void _Canbus::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		string msg = this->getName();
		if (m_pIO->bOpen())
			msg += ": CONNECTED";
		else
			msg += ": Not connected";

		((_Console *)pConsole)->addMsg(msg);
	}

}
