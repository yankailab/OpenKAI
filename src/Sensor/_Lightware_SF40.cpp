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
	m_sportName = "";
	m_pSerialPort = NULL;
	m_baudRate = 115200;

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

}

_Lightware_SF40::~_Lightware_SF40()
{
}

void _Lightware_SF40::close()
{
	if (m_pSerialPort)
	{
		m_pSerialPort->Close();
		delete m_pSerialPort;
		m_pSerialPort = NULL;
	}
	printf("Serial port closed.\n");
}

bool _Lightware_SF40::init(Config* pConfig)
{
	CHECK_F(!this->_ThreadBase::init(pConfig));
	pConfig->m_pInst = this;

	F_INFO(pConfig->v("offsetAngle", &m_offsetAngle));
	F_INFO(pConfig->v("minDist", &m_minDist));
	F_INFO(pConfig->v("maxDist", &m_maxDist));
	F_INFO(pConfig->v("showScale", &m_showScale));

	F_ERROR_F(pConfig->v("nDiv", &m_nDiv));
    m_dAngle = DEG_AROUND / m_nDiv;
    m_pDist = new double[m_nDiv+1];

	F_ERROR_F(pConfig->v("portName", &m_sportName));
	F_ERROR_F(pConfig->v("baudrate", &m_baudRate));
	m_pSerialPort = new SerialPort();

	F_INFO(pConfig->v("mwlX", &m_mwlX));
	F_INFO(pConfig->v("mwlY", &m_mwlY));
	m_pX = new Filter();
	m_pY = new Filter();
	m_pX->startMedian(m_mwlX);
	m_pY->startMedian(m_mwlY);

	m_pSF40sender = new _Lightware_SF40_sender();
	m_pSF40sender->m_pSerialPort = m_pSerialPort;
	m_pSF40sender->m_dAngle = m_dAngle;
	CHECK_F(!m_pSF40sender->init(pConfig));

	return true;
}

bool _Lightware_SF40::link(void)
{
	NULL_F(m_pConfig);

	string iName = "";
	F_ERROR_F(m_pConfig->v("_Universe", &iName));
	m_pUniverse = (_Universe*) (m_pConfig->root()->getChildInstByName(&iName));

	//TODO: link my variables to Automaton

	printf("_Lightware_SF40 link complete\n");

	return true;
}

bool _Lightware_SF40::start(void)
{
	CHECK_F(!m_pSF40sender->start());

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
		if (m_sportName == "")
		{
			this->sleepThread(1, 0);
			continue;
		}

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

		this->autoFPSfrom();

		updateLidar();
		updatePosition();

		this->autoFPSto();
	}

}

void _Lightware_SF40::updateLidar(void)
{
	F_(read());

	string str;
    istringstream sStr;
    string recv = m_strRecv;
    m_strRecv.clear();

	//separate the command part
    vector<string> vStr;
    vStr.clear();
    sStr.str(recv);
    while (getline(sStr, str, ' '))
    {
        vStr.push_back(str);
    }
    CHECK_(vStr.size()<2);

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
    CHECK_(vCmd.size()<2);
    double angle = atof(vCmd.at(1).c_str());

	//find the result
    vector<string> vResult;
    sStr.clear();
    sStr.str(result);
    while (getline(sStr, str, ','))
    {
        vResult.push_back(str);
    }
    CHECK_(vResult.size()<1);
    double dist = atof(vResult.at(0).c_str());

//    printf("angle:%f, dist:%f\n",angle,dist);

    int iAngle = (int)(angle/m_dAngle);
    m_pDist[iAngle] = dist;

//    uint64_t timeNow = get_time_usec();
//    static uint64_t timeLast = 0;
//    if(iAngle==0)
//    {
//    	uint64_t dTime = timeNow - timeLast;
//    	timeLast = timeNow;
//    	printf("dTime:%d\n",dTime);
//
//    }

}

bool _Lightware_SF40::read(void)
{
	char buf;

	while (m_pSerialPort->Read(&buf, 1))
	{
		if(buf==0)continue;
		if(buf==CR)continue;
		if(buf==LF)return true;

		m_strRecv += buf;
	}

	return false;
}

void _Lightware_SF40::updatePosition(void)
{
	int i,nV;
	double pX=0;
	double pY=0;

	for(i=0,nV=0; i<m_nDiv; i++)
	{
		double dist = m_pDist[i];
		if(dist < m_minDist)continue;
		if(dist > m_maxDist)continue;

		double angle = m_dAngle * i * DEG_RADIAN;
		pX += (dist * sin(angle));
		pY += -(dist * cos(angle));
		nV++;
	}

	pX /= nV;
	pY /= nV;

	m_pX->input(pX);
	m_pY->input(pY);
}

void _Lightware_SF40::reqMap(void)
{
	string TM = "?TM,360,0\x0d\x0a";

	m_pSerialPort->Write((char*)TM.c_str(), TM.length());
}

bool _Lightware_SF40::draw(Frame* pFrame, iVec4* pTextPos)
{
	NULL_F(pFrame);

	putText(*pFrame->getCMat(), "Lightware_SF40 FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);
	pTextPos->m_y += pTextPos->m_w;


	putText(*pFrame->getCMat(), "Lidar POS: (" + f2str(m_pX->v()) + "," + f2str(m_pY->v()) + ")",
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);
	pTextPos->m_y += pTextPos->m_w;


	if(m_nDiv<=0)return true;

	//plotting lidar output onto screen
	Mat* pMat = pFrame->getCMat();

	int cX = pMat->cols/2;
	int cY = pMat->rows/2;

	//Plot center as vehicle position
	circle(*pMat, Point(cX,cY), 10, Scalar(0, 0, 255), 2);

	//Plot lidar result
	for(int i=0; i<m_nDiv; i++)
	{
		double dist = m_pDist[i] * m_showScale;
		double angle = m_dAngle * i * DEG_RADIAN;
		int pX = (dist * sin(angle));
		int pY = -(dist * cos(angle));

		circle(*pMat, Point(cX+pX,cY+pY), 1, Scalar(0, 255, 0), 1);
	}

	return true;
}

}
