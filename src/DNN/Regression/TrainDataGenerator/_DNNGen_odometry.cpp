/*
 * _DNNodometryTrain.cpp
 *
 *  Created on: May 17, 2017
 *      Author: yankai
 */

#include "_DNNGen_odometry.h"

#ifdef USE_ZED

namespace kai
{

_DNNGen_odometry::_DNNGen_odometry()
{
	m_pZED = NULL;
	m_nGen = 0;
	m_iGen = 0;
	m_dMinTot = 0;
	m_outDir = "";
	m_fNamePrefix = "";
	m_fNameList = "dnnOdomGen.txt";
	m_width = 398;
	m_height = 224;
	m_interval = 0;

	m_zedMinConfidence = 0;
	m_bCount = false;

	m_pPrev = NULL;
	m_pNext = NULL;
	m_pDepthPrev = NULL;
	m_pDepthNext = NULL;
	m_bResize = false;
	m_bCrop = false;

	m_pMavlink = NULL;
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;
	m_freqAtti = 0;
	m_freqGlobalPos = 0;

}

_DNNGen_odometry::~_DNNGen_odometry()
{
}

bool _DNNGen_odometry::init(void* pKiss)
{
	IF_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,outDir);
	KISSm(pK,nGen);
	KISSm(pK,zedMinConfidence);
	KISSm(pK,dMinTot);
	KISSm(pK,fNamePrefix);
	KISSm(pK,fNameList);
	KISSm(pK,interval);

	KISSm(pK,bResize);
	KISSm(pK,width);
	KISSm(pK,height);

	KISSm(pK,bCrop);
	if (m_bCrop != 0)
	{
		F_ERROR_F(pK->v("cropX", &m_cropBB.x));
		F_ERROR_F(pK->v("cropY", &m_cropBB.y));
		F_ERROR_F(pK->v("cropW", &m_cropBB.width));
		F_ERROR_F(pK->v("cropH", &m_cropBB.height));
	}

	time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char strTime[128];
    strftime(strTime, sizeof(strTime), "_%F_%H-%M-%S", tm);

	string outFile = m_outDir + m_fNameList + strTime + ".odom";
	m_ofs.open(outFile.c_str(), ios::out | ios::app);

	m_pPrev = new Frame();
	m_pNext = new Frame();
	m_pDepthPrev = new Frame();
	m_pDepthNext = new Frame();

	KISSm(pK,freqAtti);
	KISSm(pK,freqGlobalPos);

	return true;
}

bool _DNNGen_odometry::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_ZED", &iName));
	m_pZED = (_ZED*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (pK->parent()->getChildInstByName(&iName));

	return true;
}

bool _DNNGen_odometry::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _DNNGen_odometry::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		if(m_nGen > 0)
		{
			if(m_iGen >= m_nGen)
			{
				m_ofs.close();
				exit(0);
			}
		}

		mavlink();

		sample();

		this->autoFPSto();
	}
}

void _DNNGen_odometry::mavlink(void)
{
	NULL_(m_pMavlink);

	//Sending Heartbeat at 1Hz
	uint64_t tNow = get_time_usec();
	if (tNow - m_lastHeartbeat >= USEC_1SEC)
	{
//		m_pMavlink->sendHeartbeat();
		m_lastHeartbeat = tNow;
	}

	//request updates from Mavlink
	if(m_freqAtti > 0)
	{
		if(tNow - m_pMavlink->m_msg.time_stamps.attitude > USEC_1SEC)
			m_pMavlink->requestDataStream(MAV_DATA_STREAM_EXTRA1, m_freqAtti);
	}

	if(m_freqGlobalPos)
	{
		if(tNow - m_pMavlink->m_msg.time_stamps.global_position_int > USEC_1SEC)
			m_pMavlink->requestDataStream(MAV_DATA_STREAM_POSITION, m_freqGlobalPos);
	}
}

void _DNNGen_odometry::sample(void)
{
	vDouble3 vT,vR,mT,mR;
	uint64_t dT;
	Mat dM1, dM2, tM;

	NULL_(m_pZED);
	IF_(!m_pZED->isOpened());

	int zedConfidence = m_pZED->getMotionDelta(&mT, &mR, &dT);

	if(zedConfidence < m_zedMinConfidence)
	{
		//zed tracking lost, reset
		m_bCount = false;
		return;
	}

	if(!m_bCount)
	{
		m_pPrev->update(m_pZED->bgr());
		m_pDepthPrev->update(m_pZED->depthNorm());
		m_mavPrev = m_pMavlink->m_msg;
		m_bCount = true;
		return;
	}

	m_pNext->update(m_pZED->bgr());
	m_pDepthNext->update(m_pZED->depthNorm());
	m_mavNext = m_pMavlink->m_msg;

	Mat* pM1 = m_pPrev->getCMat();
	Mat* pM2 = m_pNext->getCMat();
	Mat* pD1 = m_pDepthPrev->getCMat();
	Mat* pD2 = m_pDepthNext->getCMat();

	m_bCount = false;

	//validate
	IF_(pM1->empty());
	IF_(pM1->empty());
	IF_(pD1->empty());
	IF_(pD2->empty());

	IF_(pM1->rows != pM2->rows);
	IF_(pM1->cols != pM2->cols);

    vT.x = mT.z;	//forward
    vT.y = mT.x;	//right
    vT.z = mT.y;	//down
    vR.x = mR.z;	//roll
    vR.y = mR.x;	//pitch
    vR.z = mR.y;	//yaw

    double dTot = abs(vT.x) + abs(vT.y) + abs(vT.z) + abs(vR.x) + abs(vR.y) + abs(vR.z);
    IF_(dTot < m_dMinTot);

    //reverse to make nearer pixel opaque and far pixel transparent
    *pD1 = 255 - *pD1;
    *pD2 = 255 - *pD2;

    Mat Mchan[3];
    vector<Mat> pChannels;

    cv::split(*pM1,Mchan);
	pChannels.push_back(Mchan[0]);
	pChannels.push_back(Mchan[1]);
	pChannels.push_back(Mchan[2]);
	pChannels.push_back(*pD1);
	merge(pChannels, dM1);

	pChannels.clear();
    cv::split(*pM2,Mchan);
	pChannels.push_back(Mchan[0]);
	pChannels.push_back(Mchan[1]);
	pChannels.push_back(Mchan[2]);
	pChannels.push_back(*pD2);
	merge(pChannels, dM2);

	//resize
	if(m_bResize)
	{
		cv::resize(dM1, tM, Size(m_width,m_height), 0, 0, INTER_LINEAR);
		dM1 = tM;
		cv::resize(dM2, tM, Size(m_width,m_height), 0, 0, INTER_LINEAR);
		dM2 = tM;
	}

    //crop
	if(m_bCrop)
	{
		tM = Mat(dM1, m_cropBB);
		dM1 = tM;
		tM = Mat(dM2, m_cropBB);
		dM2 = tM;
	}

	//save into list and file
	time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char strTime[128];
    strftime(strTime, sizeof(strTime), "_%F_%H-%M-%S_", tm);
    string fName = m_fNamePrefix + strTime + li2str(get_time_usec());
	m_ofs << fName << "\t" << vT.x << "\t" << vT.y << "\t" << vT.z
			 	   << "\t" << vR.x << "\t" << vR.y << "\t" << vR.z
				   << "\t" << zedConfidence << "\t" << dT
				   << "\t" << m_pZED->m_depthNormInt.x << "\t" << m_pZED->m_depthNormInt.y
				   << "\t" << m_mavPrev.attitude.yaw
				   << "\t" << m_mavPrev.attitude.pitch
				   << "\t" << m_mavPrev.attitude.roll							//prev atti from AP
				   << "\t" << (m_mavPrev.global_position_int.alt * 0.001)		//prev height from AP
				   << "\t" << m_mavNext.attitude.yaw
				   << "\t" << m_mavNext.attitude.pitch
				   << "\t" << m_mavNext.attitude.roll							//next atti from AP
				   << "\t" << (m_mavNext.global_position_int.alt * 0.001)		//next height from AP
				   << endl;

	imwrite(m_outDir + fName + "_a.png", dM1);
	imwrite(m_outDir + fName + "_b.png", dM2);

	m_iGen++;
	LOG_I("Generated: "<< m_iGen);

	if(m_interval>0)
	{
		::sleep(m_interval);
	}

}

bool _DNNGen_odometry::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	IF_F(pMat->empty());

	return true;
}

}
#endif

