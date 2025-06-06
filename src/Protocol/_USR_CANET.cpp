#include "_USR_CANET.h"

namespace kai
{

	_USR_CANET::_USR_CANET()
	{
		m_pIO = nullptr;
		m_nCanData = 0;
		m_recvMsg.init();
	}

	_USR_CANET::~_USR_CANET()
	{
	}

	int _USR_CANET::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		m_nCanData = 0;
		while (1)
		{
			IF__(m_nCanData >= N_CANDATA, OK_ERR_INVALID_VALUE);
			Kiss *pAddr = pK->child(m_nCanData);
			if (pAddr->empty())
				break;

			m_pCanData[m_nCanData].init();
			IF__(!pAddr->v("addr", (int *)&(m_pCanData[m_nCanData].m_addr)), OK_ERR_NOT_FOUND);
			m_nCanData++;
		}

		string n;
		n = "";
		pK->v("_IObase", &n);
		m_pIO = (_SerialPort *)(pK->findModule(n));
		NULL__(m_pIO, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _USR_CANET::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _USR_CANET::update(void)
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
			m_pT->autoFPS();

			//Handling incoming messages
			recv();

		}
	}

	bool _USR_CANET::recv()
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

	void _USR_CANET::recvMsg(void)
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

	uint8_t *_USR_CANET::get(unsigned long addr)
	{
		for (int i = 0; i < m_nCanData; i++)
		{
			CAN_DATA *pCan = &m_pCanData[i];
			IF_CONT(pCan->m_addr != addr);

			return pCan->m_pData;
		}

		return NULL;
	}

	void _USR_CANET::send(unsigned long addr, unsigned char len, unsigned char *pData)
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

	void _USR_CANET::pinOut(uint8_t pin, uint8_t output)
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

	void _USR_CANET::console(void *pConsole)
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
