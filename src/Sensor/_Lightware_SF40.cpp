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
	m_pX = NULL;
	m_pY = NULL;

	m_pFileOut = NULL;
	m_pFileIn = NULL;
	m_inputMode = SF40_UART;

}

_Lightware_SF40::~_Lightware_SF40()
{
	if (m_pSerialPort)
	{
		m_pSerialPort->Close();
		delete m_pSerialPort;
		m_pSerialPort = NULL;
	}

	if(m_pFileOut)
	{
		m_pFileOut->close();
		delete m_pFileOut;
	}

	if(m_pFileIn)
	{
		m_pFileIn->close();
		delete m_pFileIn;
	}

	DEL(m_pX);
	DEL(m_pY);
}

bool _Lightware_SF40::init(void* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));

	string inputMode = "uart";
	string fileName = "";
	F_INFO(pK->v("inputMode", &inputMode));

	//common in all input modes
	F_INFO(pK->v("offsetAngle", &m_offsetAngle));
	F_INFO(pK->v("minDist", &m_minDist));
	F_INFO(pK->v("maxDist", &m_maxDist));
	F_INFO(pK->v("showScale", &m_showScale));

	F_ERROR_F(pK->v("nDiv", &m_nDiv));
    m_dAngle = DEG_AROUND / m_nDiv;
    m_pDist = new double[m_nDiv+1];

	F_INFO(pK->v("mwlX", &m_mwlX));
	F_INFO(pK->v("mwlY", &m_mwlY));
	m_pX = new Filter();
	m_pY = new Filter();
	m_pX->startMedian(m_mwlX);
	m_pY->startMedian(m_mwlY);

	if(inputMode=="uart")
	{
		m_inputMode = SF40_UART;
		F_ERROR_F(pK->v("portName", &m_sportName));
		F_ERROR_F(pK->v("baudrate", &m_baudRate));
		m_pSerialPort = new SerialPort();

		m_pSF40sender = new _Lightware_SF40_sender();
		m_pSF40sender->m_pSerialPort = m_pSerialPort;
		m_pSF40sender->m_dAngle = m_dAngle;
		CHECK_F(!m_pSF40sender->init(pKiss));
	}
	else if(inputMode=="file")
	{
		m_inputMode = SF40_FILE;

		fileName = "";
		F_ERROR_F(pK->v("fileIn", &fileName));
		fileName = presetDir + fileName;
		printf("SF40 input file: %s\n",fileName.c_str());

		m_pFileIn = new FileIO();
		F_ERROR_F(m_pFileIn->open(&fileName));
	}

	//log file output
	fileName = "";
	F_INFO(pK->v("fileOut", &fileName));
	if(!fileName.empty())
	{
		char date[64];
		time_t t = time(NULL);
		strftime(date, sizeof(date), "_%Y-%m-%d_%a_%H-%M-%S.sf40log", localtime(&t));
		fileName = presetDir + fileName + date;

		printf("SF40 output file: %s\n",fileName.c_str());
		m_pFileOut = new FileIO();
		F_ERROR_F(m_pFileOut->open(&fileName));
	}

	return true;
}

bool _Lightware_SF40::link(void)
{
	NULL_F(m_pKiss);
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";
	F_INFO(pK->v("_Universe", &iName));
	m_pUniverse = (_Universe*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _Lightware_SF40::start(void)
{
	if(m_inputMode==SF40_UART)
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

bool _Lightware_SF40::connect(void)
{
	if(m_inputMode == SF40_UART)
	{
		if (m_sportName == "")
		{
			this->sleepThread(USEC_1SEC);
			return false;
		}

		if (!m_pSerialPort->IsConnected())
		{
			if (m_pSerialPort->Open((char*) m_sportName.c_str()))
			{
				LOG(INFO)<< "Serial port: "+m_sportName+" connected";
			}
			else
			{
				this->sleepThread(USEC_1SEC);
				return false;
			}

			if (!m_pSerialPort->Setup(m_baudRate, 8, 1, false, false))
			{
				LOG(INFO)<< "Serial port: "+m_sportName+" could not be configured";
				m_pSerialPort->Close();
				this->sleepThread(USEC_1SEC);
				return false;
			}
		}

		return true;
	}

	return true;
}

void _Lightware_SF40::update(void)
{
	while (m_bThreadON)
	{
		if(!connect())continue;

		this->autoFPSfrom();

		updateLidar();
		updatePosition();

		this->autoFPSto();
	}

}

void _Lightware_SF40::updateLidar(void)
{
	if(m_inputMode != SF40_NET)
	{
		F_(readLine());
	}

	//output to file
	if(m_pFileOut)
	{
		m_pFileOut->write((char*)m_strRecv.c_str(), m_strRecv.length());
		m_pFileOut->writeCRLF();
	}

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

    int iAngle = (int)(angle/m_dAngle);
    m_pDist[iAngle] = dist;

    m_strRecv.clear();
}

bool _Lightware_SF40::readLine(void)
{
	if(m_inputMode == SF40_UART)
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
	else if(m_inputMode == SF40_FILE)
	{
		string* pLine = m_pFileIn->readLine();
		NULL_F(pLine);

		m_strRecv = *pLine;
		int iLen = m_strRecv.length()-1;
		if(m_strRecv.at(iLen)==LF)
		{
			m_strRecv.erase(iLen,1);
		}
		iLen = m_strRecv.length()-1;
		if(m_strRecv.at(iLen)==CR)
		{
			m_strRecv.erase(iLen,1);
		}
		return true;
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
		double angle = (m_dAngle * i + m_offsetAngle) * DEG_RADIAN;
		int pX = (dist * sin(angle));
		int pY = -(dist * cos(angle));

		circle(*pMat, Point(cX+pX,cY+pY), 1, Scalar(0, 255, 0), 1);
	}

	return true;
}

}
