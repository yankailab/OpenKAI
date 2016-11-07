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
	m_lastReq = 0;

	for(int i=0;i<DIV_AROUND;i++)
	{
		m_pDist[i] = 0.0;
	}

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

	F_ERROR_F(pConfig->v("portName", &m_sportName));
	F_ERROR_F(pConfig->v("baudrate", &m_baudRate));
	m_pSerialPort = new SerialPort();

	F_INFO(pConfig->v("mwlX", &m_mwlX));
	F_INFO(pConfig->v("mwlY", &m_mwlY));
	m_pX = new Filter();
	m_pY = new Filter();
	m_pX->startMedian(m_mwlX);
	m_pY->startMedian(m_mwlY);

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
		//Establish serial connection
		if (m_sportName == "")
		{
			this->sleepThread(1, 0);
			continue;
		}

		//Try to open and setup the serial port
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

		//Regular update loop
		this->autoFPSfrom();

		updateLidar();

		this->autoFPSto();
	}

}

void _Lightware_SF40::updateLidar(void)
{
	uint64_t timeNow = get_time_usec();
	if(timeNow - m_lastReq > 100000)
	{
		reqMap();
		m_lastReq = timeNow;
	}

	F_(read());

	//trim the received line
	std::string::size_type k;

	k = m_strRecv.find(CR);
	while (k != std::string::npos)
	{
		m_strRecv.erase(k,1);
		k = m_strRecv.find(CR);
	}

	k = m_strRecv.find(LF);
	while (k != std::string::npos)
	{
		m_strRecv.erase(k,1);
		k = m_strRecv.find(LF);
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

    if(vStr.size()<2)return;

    string cmd = vStr.at(0);
    string result = vStr.at(1);

	//separate the result part
    vector<string> vResult;
    sStr.clear();
    sStr.str(result);
    while (getline(sStr, str, ','))
    {
        vResult.push_back(str);
    }

    m_nDiv = atoi(vResult.at(0).c_str());
    m_dAngle = DEG_AROUND / m_nDiv;

    if(m_nDiv > vResult.size()-1)m_nDiv = vResult.size()-1;

    for(int i=0; i<m_nDiv; i++)
    {
    	m_pDist[i] = atof(vResult.at(i+1).c_str());

//    	printf("%f, ",m_pDist[i]);
    }

//	printf("\n");

    m_strRecv.clear();


    //TODO: change TM to LD for less data over UART

}

bool _Lightware_SF40::read(void)
{
	char buf;

	while (m_pSerialPort->Read(&buf, 1))
	{
		if(buf==0)continue;

		m_strRecv += buf;

		if(buf==LF)
		{
			return true;
		}
	}

	return false;
}

void _Lightware_SF40::reqMap(void)
{
	string TM = "?TM,360,0\x0d\x0a";

	m_pSerialPort->Write((char*)TM.c_str(), TM.length());
}

void _Lightware_SF40::write(void)
{
	char buf[300];

	// Translate message to buffer
	unsigned int len = 0;

	// Write buffer to serial port, locks port while writing
	m_pSerialPort->Write(buf, len);

}

bool _Lightware_SF40::draw(Frame* pFrame, iVec4* pTextPos)
{
	NULL_F(pFrame);

	putText(*pFrame->getCMat(), "Lightware_SF40 FPS: " + i2str(getFrameRate()),
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
		double angle = m_dAngle * i;
		int pX = (dist * cos(angle*DEG_RADIAN));
		int pY = (dist * sin(angle*DEG_RADIAN));

		circle(*pMat, Point(cX+pX,cY+pY), 1, Scalar(0, 255, 0), 1);
	}

	return true;
}

}
