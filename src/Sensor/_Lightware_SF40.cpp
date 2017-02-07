/*
 *  Created on: Nov 4, 2016
 *      Author: yankai
 */
#include "_Lightware_SF40.h"

namespace kai
{

_Lightware_SF40::_Lightware_SF40()
{
	m_pIn = NULL;
	m_hdg = 0.0;
	m_offsetAngle = 0.0;
	m_nDiv = 0;
	m_dAngle = 0;
	m_minDist = 1.0;
	m_maxDist = 50.0;
	m_showScale = 1.0;	//1m = 1pixel;
	m_strRecv = "";
	m_pSF40sender = NULL;
	m_MBS = 0;

	m_pDist = NULL;
	m_dPos.init();
	m_lastPos.init();
	m_varianceLim = 10;
	m_diffMax = 1.0;
	m_diffMin = 0.25;

	m_tStartUp = 0;
}

_Lightware_SF40::~_Lightware_SF40()
{
	DEL(m_pSF40sender);
	DEL(m_pDist);

	if (m_pIn)
	{
		m_pIn->close();
		delete m_pIn;
		m_pIn = NULL;
	}
}

bool _Lightware_SF40::init(void* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));

	//common in all input modes
	F_INFO(pK->v("offsetAngle", &m_offsetAngle));
	F_INFO(pK->v("minDist", &m_minDist));
	F_INFO(pK->v("maxDist", &m_maxDist));
	F_INFO(pK->v("showScale", &m_showScale));
	F_INFO(pK->v("MBS", (int* )&m_MBS));
	F_INFO(pK->v("varianceLim", &m_varianceLim));
	F_INFO(pK->v("diffMax", &m_diffMax));
	F_INFO(pK->v("diffMin", &m_diffMin));

	F_ERROR_F(pK->v("nDiv", &m_nDiv));
	m_dAngle = DEG_AROUND / m_nDiv;

	Kiss* pCC;
	string param;
	int i;

	//filter
	pCC = pK->o("medianFilter");
	CHECK_F(pCC->empty());
	m_pDist = new Median[m_nDiv];
	for (i = 0; i < m_nDiv; i++)
	{
		CHECK_F(!m_pDist[i].init(pCC));
	}

	//input
	pCC = pK->o("input");
	CHECK_F(pCC->empty());
	m_pIn = new SerialPort();
	CHECK_F(!m_pIn->init(pCC));

	m_pSF40sender = new _Lightware_SF40_sender();
	m_pSF40sender->m_pSerialPort = (SerialPort*) m_pIn;
	m_pSF40sender->m_dAngle = m_dAngle;
	CHECK_F(!m_pSF40sender->init(pKiss));

	return true;
}

bool _Lightware_SF40::link(void)
{
	CHECK_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _Lightware_SF40::start(void)
{
	if (m_pIn->type() == serialport)
	{
		CHECK_F(!m_pSF40sender->start());
	}

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

void _Lightware_SF40::update(void)
{
	while (m_bThreadON)
	{
		if (!m_pIn->isOpen())
		{
			if (!m_pIn->open())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
			m_tStartUp = get_time_usec();

			if (m_pIn->type() == serialport)
				m_pSF40sender->MBS(m_MBS);
		}

		this->autoFPSfrom();

		if (updateLidar())
		{
			updateRelativePos();
		}

		this->autoFPSto();
	}

}

bool _Lightware_SF40::updateLidar(void)
{
	CHECK_F(!readLine());

	string str;
	istringstream sStr;

	//separate the command part
	vector<string> vStr;
	vStr.clear();
	sStr.str(m_strRecv);
	while (getline(sStr, str, ' '))
	{
		vStr.push_back(str);
	}
	if (vStr.size() < 2)
	{
		m_strRecv.clear();
		return false;
	}

	string cmd = vStr.at(0);
	string result = vStr.at(1);

	//find the angle from cmd
	vector<string> vCmd;
	sStr.clear();
	sStr.str(cmd);
	while (getline(sStr, str, ','))
	{
		vCmd.push_back(str);
	}
	if (vCmd.size() < 2)
	{
		m_strRecv.clear();
		return false;
	}
	double angle = atof(vCmd.at(1).c_str());

	//find the result
	vector<string> vResult;
	sStr.clear();
	sStr.str(result);
	while (getline(sStr, str, ','))
	{
		vResult.push_back(str);
	}
	if (vResult.size() < 1)
	{
		m_strRecv.clear();
		return false;
	}

	double dist = atof(vResult.at(0).c_str());

	angle += m_hdg + m_offsetAngle;
	while (angle >= DEG_AROUND)
		angle -= DEG_AROUND;

	int iAngle = (int) (angle / m_dAngle);
	if (iAngle >= m_nDiv)
	{
		m_strRecv.clear();
		return false;
	}

	m_pDist[iAngle].input(dist);
	m_strRecv.clear();

	return true;
}

bool _Lightware_SF40::readLine(void)
{
	char buf;
	while (m_pIn->read((uint8_t*) &buf, 1) > 0)
	{
		if (buf == 0)
			continue;
		if (buf == LF || buf == CR)
		{
			if (m_strRecv.empty())
				continue;

			return true;
		}

		m_strRecv += buf;
	}

	return false;
}

void _Lightware_SF40::updateRelativePos(void)
{
	int i;
	ZONE pZone[4];
	pZone[0].init();
	pZone[1].init();
	pZone[2].init();
	pZone[3].init();
	int nZone = m_nDiv/4;

	for (i = 0; i < m_nDiv; i++)
	{
		Median* pD = &m_pDist[i];

		CHECK_CONT(pD->variance() > m_varianceLim);

		double diff = pD->diff();
		CHECK_CONT(diff < m_diffMin);
		CHECK_CONT(diff > m_diffMax);

		double dist = pD->v();
		CHECK_CONT(dist < m_minDist);
		CHECK_CONT(dist > m_maxDist);

		double rad = m_dAngle * i * DEG_RADIAN;
		double x = (dist * sin(rad));
		double y = -(dist * cos(rad));

		ZONE* pZ = &pZone[i/nZone];
		pZ->m_x += x;
		pZ->m_y += y;
		pZ->m_n += 1.0;
	}

	pZone[0].avr();
	pZone[1].avr();
	pZone[2].avr();
	pZone[3].avr();

	double pX = pZone[0].m_x + pZone[1].m_x + pZone[2].m_x + pZone[3].m_x;
	double pY = pZone[0].m_y + pZone[1].m_y + pZone[2].m_y + pZone[3].m_y;
	pX *= 0.25;
	pY *= 0.25;

	//update current pos
	m_dPos.m_x += pX - m_lastPos.m_x;
	m_dPos.m_y += pY - m_lastPos.m_y;

	m_lastPos.m_x = pX;
	m_lastPos.m_y = pY;

}

void _Lightware_SF40::setHeading(double hdg)
{
	m_hdg = hdg;
}

vDouble2 _Lightware_SF40::getDiffRelativePos(void)
{
	vDouble2 dPos = m_dPos;
	m_dPos.init();

	return dPos;
}

void _Lightware_SF40::reset(void)
{
	for (int i = 0; i < m_nDiv; i++)
	{
		m_pDist->reset();
	}
	m_dPos.init();
}

bool _Lightware_SF40::draw(void)
{
	CHECK_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	string msg;

	pWin->tabNext();
	msg = "dPos: dX=" + f2str(m_dPos.m_x) + ", dY=" + f2str(m_dPos.m_y);
	pWin->addMsg(&msg);

	if (m_pIn)
		m_pIn->draw();

	pWin->tabPrev();
	CHECK_T(m_nDiv <= 0);

	//Plot center as vehicle position
	Point pCenter(pMat->cols / 2, pMat->rows / 2);
	circle(*pMat, pCenter, 10, Scalar(0, 0, 255), 2);

	//Plot lidar result
	for (int i = 0; i < m_nDiv; i++)
	{
		Median* pD = &m_pDist[i];
		CHECK_CONT(pD->variance() > m_varianceLim);

		double diff = pD->diff();
		CHECK_CONT(diff < m_diffMin);
		CHECK_CONT(diff > m_diffMax);

		double dist = pD->v();
		CHECK_CONT(dist < m_minDist);
		CHECK_CONT(dist > m_maxDist);

		dist *= m_showScale;
		double angle = m_dAngle * i * DEG_RADIAN;
		int pX = (dist * sin(angle));
		int pY = -(dist * cos(angle));

		Scalar col = Scalar(255,255,255);
		circle(*pMat, pCenter + Point(pX, pY), 1, col, 2);
	}

	return true;
}

}
