/*
 *  Created on: Nov 4, 2016
 *      Author: yankai
 */
#include "_Lightware_SF40.h"

namespace kai
{

_Lightware_SF40::_Lightware_SF40()
{
	_ThreadBase();

	m_pUniverse = NULL;
	m_pInput = NULL;
	m_pOutput = NULL;

	m_offsetAngle = 0.0;
	m_nDiv = 0;
	m_dAngle = 0;
	m_minDist = 1.0;
	m_maxDist = 50.0;
	m_showScale = 1.0;	//1m = 1pixel;
	m_mwlX = 3;
	m_mwlY = 3;
	m_strRecv = "";
	m_pSF40sender = NULL;
	m_pDist = NULL;
	m_pX = NULL;
	m_pY = NULL;
}

_Lightware_SF40::~_Lightware_SF40()
{
	if (m_pInput)
	{
		m_pInput->close();
		delete m_pInput;
		m_pInput = NULL;
	}

	if (m_pOutput)
	{
		m_pOutput->close();
		delete m_pOutput;
		m_pOutput = NULL;
	}

	DEL(m_pX);
	DEL(m_pY);
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

	F_ERROR_F(pK->v("nDiv", &m_nDiv));
	m_dAngle = DEG_AROUND / m_nDiv;
	m_pDist = new double[m_nDiv + 1];

	F_INFO(pK->v("mwlX", &m_mwlX));
	F_INFO(pK->v("mwlY", &m_mwlY));
	m_pX = new Filter();
	m_pY = new Filter();
	m_pX->startMedian(m_mwlX);
	m_pY->startMedian(m_mwlY);

	//IO
	Kiss* pCC;
	string param = "";

	pCC = pK->o("input");
	CHECK_F(pCC->empty());
	F_ERROR_F(pCC->v("class", &param));
	if (param == "SerialPort")
	{
		m_pInput = new SerialPort();
		CHECK_F(m_pInput->init(pCC));

		m_pSF40sender = new _Lightware_SF40_sender();
		m_pSF40sender->m_pSerialPort = (SerialPort*)m_pInput;
		m_pSF40sender->m_dAngle = m_dAngle;
		CHECK_F(!m_pSF40sender->init(pKiss));
	}
	else if (param == "File")
	{
		m_pInput = new File();
		F_ERROR_F(m_pInput->init(pCC));
	}

	pCC = pK->o("output");
	CHECK_T(pCC->empty());
	F_ERROR_F(pCC->v("class", &param));
	if (param == "SerialPort")
	{
		m_pOutput = new SerialPort();
		CHECK_F(m_pOutput->init(pCC));
	}
	else if (param == "File")
	{
		m_pOutput = new File();
		F_ERROR_F(m_pOutput->init(pCC));
	}

	return true;
}

bool _Lightware_SF40::link(void)
{
	NULL_F(m_pKiss);
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("_Universe", &iName));
	m_pUniverse = (_Universe*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _Lightware_SF40::start(void)
{
	if (m_pInput->type() == serialport)
	{
		CHECK_F(!m_pSF40sender->start());
	}

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Lightware_SF40::update(void)
{
	while (m_bThreadON)
	{
		if (!m_pInput->isOpen())
		{
			if (!m_pInput->open())
			{
				this->sleepThread(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		updateLidar();
		updatePosition();

		this->autoFPSto();
	}

}

void _Lightware_SF40::updateLidar(void)
{
	F_(readLine());

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
	CHECK_(vStr.size() < 2);

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
	CHECK_(vCmd.size() < 2);
	double angle = atof(vCmd.at(1).c_str());

	//find the result
	vector<string> vResult;
	sStr.clear();
	sStr.str(result);
	while (getline(sStr, str, ','))
	{
		vResult.push_back(str);
	}
	CHECK_(vResult.size() < 1);
	double dist = atof(vResult.at(0).c_str());

	int iAngle = (int) (angle / m_dAngle);
	m_pDist[iAngle] = dist;

	m_strRecv.clear();
}

bool _Lightware_SF40::readLine(void)
{
	char buf;
	while (m_pInput->read((uint8_t*)&buf, 1) > 0)
	{
		if (buf == 0)continue;
		if (buf == CR)continue;
		if (buf == LF)
		{
			if (m_pOutput)
			{
				m_pOutput->writeLine((uint8_t*) m_strRecv.c_str(), m_strRecv.length());
			}
			return true;
		}

		m_strRecv += buf;
	}

	return false;
}

void _Lightware_SF40::updatePosition(void)
{
	int i, nV;
	double pX = 0;
	double pY = 0;

	for (i = 0, nV = 0; i < m_nDiv; i++)
	{
		double dist = m_pDist[i];
		if (dist < m_minDist)
			continue;
		if (dist > m_maxDist)
			continue;

		double angle = (m_dAngle * i + m_offsetAngle) * DEG_RADIAN;
		pX += (dist * sin(angle));
		pY += -(dist * cos(angle));
		nV++;
	}

	pX /= nV;
	pY /= nV;

	m_pX->input(pX);
	m_pY->input(pY);

	//TODO: set new position when difference is bigger than a threshold

}

bool _Lightware_SF40::draw(Frame* pFrame, vInt4* pTextPos)
{
	NULL_F(pFrame);

	putText(*pFrame->getCMat(), "Lightware_SF40 FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);
	pTextPos->m_y += pTextPos->m_w;

	putText(*pFrame->getCMat(),
			"Lidar POS: (" + f2str(m_pX->v()) + "," + f2str(m_pY->v()) + ")",
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);
	pTextPos->m_y += pTextPos->m_w;

	if (m_nDiv <= 0)
		return true;

	//plotting lidar output onto screen
	Mat* pMat = pFrame->getCMat();

	int cX = pMat->cols / 2;
	int cY = pMat->rows / 2;

	//Plot center as vehicle position
	circle(*pMat, Point(cX, cY), 10, Scalar(0, 0, 255), 2);

	//Plot lidar result
	for (int i = 0; i < m_nDiv; i++)
	{
		double dist = m_pDist[i] * m_showScale;
		double angle = (m_dAngle * i + m_offsetAngle) * DEG_RADIAN;
		int pX = (dist * sin(angle));
		int pY = -(dist * cos(angle));

		circle(*pMat, Point(cX + pX, cY + pY), 1, Scalar(0, 255, 0), 1);
	}

	return true;
}

}
