#include "_SkydroidGimbal.h"

namespace kai
{
	_SkydroidGimbal::_SkydroidGimbal()
	{
		m_pTr = NULL;
		m_pIO = NULL;
	}

	_SkydroidGimbal::~_SkydroidGimbal()
	{
		DEL(m_pTr);
	}

	bool _SkydroidGimbal::init(void *pKiss)
	{
		IF_F(!this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
		

		pK->v("cmd", &m_cmd);

		Kiss *pKt = pK->child("threadR");
		IF_d_F(pKt->empty(), LOG_E("threadR not found"));
		m_pTr = new _Thread();
		if (!m_pTr->init(pKt))
		{
			DEL(m_pTr);
			return false;
		}

		return true;
	}

	bool _SkydroidGimbal::link(void)
	{
		IF_F(!this->_ActuatorBase::link());
		IF_F(!m_pTr->link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;
		n = "";
		F_ERROR_F(pK->v("_IObase", &n));
		m_pIO = (_IObase *)(pK->getInst(n));
		NULL_Fl(m_pIO, n + ": not found");

		return true;
	}

	bool _SkydroidGimbal::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTr);
		IF_F(!m_pT->start(getUpdate, this));
		return m_pTr->start(getUpdateR, this);
	}

	void _SkydroidGimbal::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			sendCMD();

			m_pT->autoFPSto();
		}
	}

	void _SkydroidGimbal::sendCMD(void)
	{
		NULL_(m_pIO);
		IF_(!m_pIO->bOpen());
		IF_(m_cmd.empty());

		string cmd = m_cmd;
		char crc = calcCRC(cmd.c_str(), (int)cmd.length());

		char buf[3];
		snprintf(buf, 3, "%X", crc);
		cmd += string(buf);

		m_pIO->write((uint8_t *)cmd.c_str(), (int)cmd.length());

		// int i;
		// uint16_t pChan[16];
		// int nC = m_vAxis.size();
		// for (i = 0; i < nC; i++)
		// {
		// 	ACTUATOR_AXIS *pA = &m_vAxis[i];
		// 	pChan[i] = 1500;
		// 	if (pA->m_p.bInRange(pA->m_p.m_vTarget))
		// 		pChan[i] = pA->m_p.m_vTarget;

		// 	pA->m_p.m_v = pA->m_p.m_vTarget;
		// }
	}

	char _SkydroidGimbal::calcCRC(const char *cmd, uint8_t len)
	{
		char crc = 0;
		for (int i = 0; i < len; i++)
		{
			crc += cmd[i];
		}
		return crc;
	}

	void _SkydroidGimbal::updateR(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			while (readCMD())
			{
				handleCMD();
			}

			m_pT->autoFPSto();
		}
	}

	bool _SkydroidGimbal::readCMD(void)
	{
		NULL_F(m_pIO);
		IF_F(!m_pIO->bOpen());

		uint8_t inByte;
		int byteRead;

		while ((byteRead = m_pIO->read(&inByte, 1)) > 0)
		{
		}

		return false;
	}

	void _SkydroidGimbal::handleCMD(void)
	{
	}

	void _SkydroidGimbal::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ActuatorBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		if (!m_pIO->bOpen())
		{
			pC->addMsg("Not Connected", 1);
			return;
		}

		//		pC->addMsg("nCMD = " + i2str(m_nCMDrecv), 1);
	}

}
