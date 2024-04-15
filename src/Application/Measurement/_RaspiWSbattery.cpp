/*
 * _RaspiWSbattery.cpp
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#include "_RaspiWSbattery.h"

namespace kai
{

	_RaspiWSbattery::_RaspiWSbattery()
	{
		m_cmdBatt = "python3 INA219.py";
		m_battV = 0;
		m_battA = 0;
		m_battW = 0;
		m_battP = 100;
		m_battShutdown = 10;
	}

	_RaspiWSbattery::~_RaspiWSbattery()
	{
	}

	bool _RaspiWSbattery::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("cmdBatt", &m_cmdBatt);
		pK->v("battShutdown", &m_battShutdown);

		return true;
	}

	bool _RaspiWSbattery::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _RaspiWSbattery::check(void)
	{
		return 0;
	}

	void _RaspiWSbattery::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateBatt();

			m_pT->autoFPSto();
		}
	}

	bool _RaspiWSbattery::updateBatt(void)
	{
		// check battery
		FILE *pFr = popen(m_cmdBatt.c_str(), "r");
		IF_F(pFr <= 0);

		char pB[256];
		float pBatt[4]; //voltage, current, power, percent
		int i;
		for (i = 0; i < 4 && fgets(pB, sizeof(pB), pFr) != NULL; i++)
			pBatt[i] = atof(pB);

		IF_F(i < 4);

		m_battV = pBatt[0];
		m_battA = pBatt[1];
		m_battW = pBatt[2];
		m_battP = pBatt[3];
		pclose(pFr);

		LOG_I("Battery: V=" + f2str(m_battV) +
			  ", A=" + f2str(m_battA) +
			  ", W=" + f2str(m_battW) +
			  ", P=" + f2str(m_battP));

		if (m_battP < m_battShutdown)
			system("shutdown -P now");

		return true;
	}

	void _RaspiWSbattery::draw(void* pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);

		Frame *pF = (Frame*)pFrame;
		Mat *pMw = pF->m();
		IF_(pMw->empty());
		cv::Ptr<freetype::FreeType2> pFt = pWin->getFont();
		NULL_(pFt);

		Scalar col = (m_battP > m_battShutdown * 1.5) ? Scalar(255, 255, 255) : Scalar(0, 0, 255);
		string sB = "Bat." + i2str((int)m_battP) + "%";

		Point pt;
		pt.x = 510;
		pt.y = pMw->rows - 45;

		pFt->putText(*pMw, sB,
					   pt,
					   40,
					   col,
					   -1,
					   cv::LINE_AA,
					   false);
	}

	void _RaspiWSbattery::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
	}

}
