/*
 *  Created on: July 15, 2020
 *      Author: yankai
 */
#include "_TOFsense.h"

namespace kai
{

	_TOFsense::_TOFsense()
	{
		m_pIO = NULL;
		m_nDiv = 1;
	}

	_TOFsense::~_TOFsense()
	{
	}

	bool _TOFsense::init(void *pKiss)
	{
		IF_F(!this->_DistSensorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;

		n = "";
		F_ERROR_F(pK->v("_IOBase", &n));
		m_pIO = (_IOBase *)(pK->getInst(n));
		IF_Fl(!m_pIO, n + " not found");

		m_frame.init(16);
		m_bReady = 1;

		return true;
	}

	bool _TOFsense::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _TOFsense::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_pIO)
			{
				m_pT->sleepT(SEC_2_USEC);
				continue;
			}

			if (!m_pIO->isOpen())
			{
				m_pT->sleepT(SEC_2_USEC);
				continue;
			}

			m_pT->autoFPSfrom();

			while (readCMD())
			{
				handleCMD();
			}

			m_pT->autoFPSto();
		}
	}

	bool _TOFsense::readCMD(void)
	{
		uint8_t b;
		int nB;

		while ((nB = m_pIO->read(&b, 1)) > 0)
		{
			if (m_frame.m_header != 0)
			{
				m_frame.m_pB[m_frame.m_iB] = b;
				m_frame.m_iB++;

				if (m_frame.m_iB == 2)
				{
					//				m_frame.m_nPayload = ;
				}
				else if (m_frame.m_iB >= 16)
				{
					return true;
				}
			}
			else if (b == NLINK_BEGIN)
			{
				m_frame.m_header = b;
				m_frame.m_pB[0] = b;
				m_frame.m_iB = 1;
			}
		}

		return false;
	}

	void _TOFsense::handleCMD(void)
	{
		uint8_t cs = verifyCheckSum(m_frame.m_pB, 16);
		TOFSENSE_SENSOR rf;
		memcpy(&rf, m_frame.m_pB, 16);
		m_frame.reset();
		IF_(!cs);

		this->input(rf.id,
					int24Value((uint8_t *)&rf.distance) * 0.001,
					rf.signalStrength);
	}

	int32_t _TOFsense::int24Value(uint8_t *pD)
	{
		NULL__(pD, -1);

		return (int32_t)(pD[0] << 8 | pD[1] << 16 | pD[2] << 24) / 256;
	}

	uint8_t _TOFsense::verifyCheckSum(const uint8_t *data, size_t dataLength)
	{
		uint8_t sum = 0;
		for (size_t i = 0; i < dataLength - 1; i++)
		{
			sum += data[i];
		}
		return sum == data[dataLength - 1];
	}

	DIST_SENSOR_TYPE _TOFsense::type(void)
	{
		return ds_TOFsense;
	}

	void _TOFsense::console(void *pConsole)
	{
#ifdef WITH_UI
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		string msg;
		msg += "nDiv=" + i2str(m_nDiv);
		pC->addMsg(msg);

		int i;
		msg = "";
		for (i = 0; i < m_nDiv; i++)
		{
			DIST_SENSOR_DIV *pD = &m_pDiv[i];
			msg += " | " + f2str(pD->d()) + " (" + f2str(pD->a()) + ")";
		}
		msg += " |";
		pC->addMsg(msg);
#endif
	}

	void _TOFsense::draw(void* pFrame)
	{
#ifdef WITH_UI
#ifdef USE_OPENCV
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pFrame;
		Frame *pF = pWin->getNextFrame();
		NULL_(pF);
		Mat *pM = pF->m();
		IF_(pM->empty());

		Point pCenter(pM->cols * 0.5, pM->rows * 0.5);
		Scalar col = Scalar(0, 255, 0);
		Scalar colD = Scalar(0, 0, 255);
		float rMax = m_rMax * m_showScale;

		for (int i = 0; i < m_nDiv; i++)
		{
			float radFrom = (i * m_dDeg + m_showDegOffset) * DEG_2_RAD;
			float radTo = ((i + 1) * m_dDeg + m_showDegOffset) * DEG_2_RAD;
			float d = m_pDiv[i].d() * m_showScale;

			vDouble2 pFrom, pTo;
			pFrom.x = sin(radFrom);
			pFrom.y = -cos(radFrom);
			pTo.x = sin(radTo);
			pTo.y = -cos(radTo);

			line(*pM, pCenter + Point(pFrom.x * d, pFrom.y * d),
				 pCenter + Point(pTo.x * d, pTo.y * d), colD, 2);
			line(*pM, pCenter + Point(pFrom.x * rMax, pFrom.y * rMax), pCenter,
				 col, 1);
			line(*pM, pCenter, pCenter + Point(pTo.x * rMax, pTo.y * rMax), col,
				 1);
		}
#endif
#endif
	}

}
