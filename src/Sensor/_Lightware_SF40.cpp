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
	m_nMeasureDiv = 0;
	m_dAngle = 0;
	m_minDist = 0.1;
	m_maxDist = 100.0;
	m_showScale = 1.0;	//1m = 1pixel;
	m_strRecv = "";
	m_pSF40sender = NULL;
	m_MBS = 0;

	m_pDistMed = NULL;
	m_pDistAvr = NULL;
	m_dPos.init();
	m_diffMax = 1.0;
	m_diffMin = 0.25;

	m_nReceived = 0;
	m_nUpdate = 0;
}

_Lightware_SF40::~_Lightware_SF40()
{
	DEL(m_pSF40sender);
	DEL(m_pDistMed);
	DEL(m_pDistAvr);

	if (m_pIn)
	{
		m_pIn->close();
		delete m_pIn;
		m_pIn = NULL;
	}
}

bool _Lightware_SF40::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	F_INFO(pK->v("minDist", &m_minDist));
	F_INFO(pK->v("maxDist", &m_maxDist));
	F_INFO(pK->v("showScale", &m_showScale));
	F_INFO(pK->v("MBS", (int* )&m_MBS));
	F_INFO(pK->v("diffMax", &m_diffMax));
	F_INFO(pK->v("diffMin", &m_diffMin));
	F_INFO(pK->v("nUpdate", &m_nUpdate));
	F_INFO(pK->v("offsetAngle", &m_offsetAngle));
	while (m_offsetAngle < 0)
		m_offsetAngle += DEG_AROUND;

	F_ERROR_F(pK->v("nDiv", &m_nDiv));
	m_nMeasureDiv = m_nDiv;
	F_ERROR_F(pK->v("nMeasureDiv", &m_nMeasureDiv));
	m_dAngle = DEG_AROUND / m_nDiv;

	Kiss* pCC;
	string param;
	int i;

	//filter
	pCC = pK->o("medianFilter");
	IF_F(pCC->empty());
	m_pDistMed = new Median[m_nDiv];
	for (i = 0; i < m_nDiv; i++)
	{
		IF_F(!m_pDistMed[i].init(pCC));
	}

	pCC = pK->o("avrFilter");
	IF_F(pCC->empty());
	m_pDistAvr = new Average[m_nDiv];
	for (i = 0; i < m_nDiv; i++)
	{
		IF_F(!m_pDistAvr[i].init(pCC));
	}

	//input
	pCC = pK->o("input");
	IF_F(pCC->empty());
	m_pIn = new SerialPort();
	IF_F(!m_pIn->init(pCC));

	m_pSF40sender = new _Lightware_SF40_sender();
	m_pSF40sender->m_pSerialPort = (SerialPort*) m_pIn;
	m_pSF40sender->m_dAngle = DEG_AROUND / m_nMeasureDiv;
	IF_F(!m_pSF40sender->init(pKiss));

	return true;
}

bool _Lightware_SF40::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _Lightware_SF40::start(void)
{
	if (m_pIn->type() == serialport)
	{
		IF_F(!m_pSF40sender->start());
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

			if (m_pIn->type() == serialport)
				m_pSF40sender->MBS(m_MBS);
		}

		this->autoFPSfrom();

		if(readLine())
		{
			if (updateLidar())
			{
				m_nReceived++;
			}

			m_strRecv.clear();
		}

		if(m_nReceived >= m_nUpdate)
		{
			m_nReceived = 0;
			updatePosDiff();
		}

		this->autoFPSto();
	}

}

bool _Lightware_SF40::updateLidar(void)
{
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
	IF_F(vStr.size() < 2);

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
	IF_F(vCmd.size() < 2);
	double angle = atof(vCmd.at(1).c_str());

	//find the result
	vector<string> vResult;
	sStr.clear();
	sStr.str(result);
	while (getline(sStr, str, ','))
	{
		vResult.push_back(str);
	}
	IF_F(vResult.size() < 1);

	double dist = atof(vResult.at(0).c_str());

	angle += m_hdg + m_offsetAngle;
	while (angle >= DEG_AROUND)
		angle -= DEG_AROUND;

	int iAngle = (int) (angle / m_dAngle);
	IF_F(iAngle >= m_nDiv);

	Median* pM = &m_pDistMed[iAngle];
	Average* pA = &m_pDistAvr[iAngle];

	pM->input(dist);
	pA->input(pM->v());

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

void _Lightware_SF40::updatePosDiff(void)
{
	int i;
	double pX = 0.0;
	double pY = 0.0;
	double nV = 0.0;
	double dRad = m_dAngle * DEG_RAD;

	for (i = 0; i < m_nDiv; i++)
	{
		Average* pA = &m_pDistAvr[i];

		double dist = pA->v();
		IF_CONT(dist < m_minDist);
		IF_CONT(dist > m_maxDist);

		double dD = pA->accumlatedDiff();
		double absDD = abs(dD);
		IF_CONT(absDD <= m_diffMin);
		IF_CONT(absDD > m_diffMax);

		double rad = dRad * i;
		pX += dD * cos(rad);
		pY += -dD * sin(rad);
		nV += 1.0;
	}

	IF_(nV < 1.0);

	//update current pos
	nV = 1.0/nV;
	m_dPos.x += pX * nV;
	m_dPos.y += pY * nV;
}

void _Lightware_SF40::setHeading(double hdg)
{
	m_hdg = hdg;
}

vDouble2 _Lightware_SF40::getPosDiff(void)
{
	vDouble2 dPos = m_dPos;
	m_dPos.init();

	return dPos;
}

void _Lightware_SF40::reset(void)
{
	for (int i = 0; i < m_nDiv; i++)
	{
		m_pDistMed[i].reset();
		m_pDistAvr[i].reset();
	}
	m_dPos.init();
}

double _Lightware_SF40::getDistance(double localAngle)
{
	double angle = m_offsetAngle - m_hdg + localAngle;

	while (angle < 0)
		angle += DEG_AROUND;

	while (angle >= DEG_AROUND)
		angle -= DEG_AROUND;

	int iAngle = (int) (angle / m_dAngle);
	if(iAngle >= m_nDiv)
	{
		return -1.0;
	}

	return m_pDistAvr[iAngle].v();
}

double _Lightware_SF40::minDist(void)
{
	return m_minDist;
}

double _Lightware_SF40::maxDist(void)
{
	return m_maxDist;
}

bool _Lightware_SF40::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	string msg;

	pWin->tabNext();
	msg = "dPos: dX=" + f2str(m_dPos.x) + ", dY=" + f2str(m_dPos.y);
	pWin->addMsg(&msg);
	msg = "nReceived=" + i2str(m_nReceived);
	pWin->addMsg(&msg);

	if (m_pIn)
		m_pIn->draw();

	pWin->tabPrev();
	IF_T(m_nDiv <= 0);

	//Plot center as vehicle position
	Point pCenter(pMat->cols / 2, pMat->rows / 2);
	circle(*pMat, pCenter, 10, Scalar(0, 0, 255), 2);

	//Plot lidar result
	for (int i = 0; i < m_nDiv; i++)
	{
		Average* pD = &m_pDistAvr[i];

		double dist = pD->v();
		IF_CONT(dist <= m_minDist);
		IF_CONT(dist > m_maxDist);

		dist *= m_showScale;
		double rad = m_dAngle * i * DEG_RAD;
		int pX = -dist * cos(rad);
		int pY = -dist * sin(rad);

		Scalar col = Scalar(255, 255, 255);
		circle(*pMat, pCenter + Point(pX, pY), 1, col, 2);
	}

	return true;
}

}
