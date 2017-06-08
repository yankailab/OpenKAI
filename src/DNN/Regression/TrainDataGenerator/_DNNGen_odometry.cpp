/*
 * _DNNodometryTrain.cpp
 *
 *  Created on: May 17, 2017
 *      Author: yankai
 */

#include "_DNNGen_odometry.h"

namespace kai
{

_DNNGen_odometry::_DNNGen_odometry()
{
	m_pZED = NULL;
	m_iStartID = 0;
	m_nGen = 0;
	m_iGen = 0;
	m_dMinTot = 0;
	m_outDir = "";
	m_format = ".png";
	m_fNamePrefix = "";
	m_fNameList = "dnnOdomGen.txt";
	m_width = 398;
	m_height = 224;

	m_zedMinConfidence = 0;
	m_bCount = false;

	m_pPrev = NULL;
	m_pNext = NULL;
	m_bCrop = false;
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
	KISSm(pK,iStartID);
	KISSm(pK,nGen);
	KISSm(pK,zedMinConfidence);
	KISSm(pK,dMinTot);
	KISSm(pK,format);
	KISSm(pK,fNamePrefix);
	KISSm(pK,fNameList);
	KISSm(pK,bCrop);
	if (m_bCrop != 0)
	{
		F_ERROR_F(pK->v("cropX", &m_cropBB.x));
		F_ERROR_F(pK->v("cropY", &m_cropBB.y));
		F_ERROR_F(pK->v("cropW", &m_cropBB.width));
		F_ERROR_F(pK->v("cropH", &m_cropBB.height));
	}

	string outFile = m_outDir + m_fNameList;
	m_ofs.open(outFile.c_str(), ios::out | ios::app);

	m_pPrev = new Frame();
	m_pNext = new Frame();

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

		sample();

		this->autoFPSto();
	}
}

void _DNNGen_odometry::sample(void)
{
	vDouble3 vT,vR,mT,mR;
	uint64_t dT;
	Mat dM, tM;

	NULL_(m_pZED);
	IF_(!m_pZED->isOpened());

	int zedConfidence = m_pZED->getMotionDelta(&mT, &mR, &dT);

	if(!m_bCount)
	{
		m_pPrev->update(m_pZED->gray());
		IF_(m_pPrev->empty());
		m_bCount = true;
		return;
	}

	m_pNext->update(m_pZED->gray());
	Mat* pM1 = m_pPrev->getCMat();
	Mat* pM2 = m_pNext->getCMat();

	m_bCount = false;

	//validate
	IF_(pM1->empty());
	IF_(pM1->empty());
	IF_(pM1->rows != pM2->rows);
	IF_(pM1->cols != pM2->cols);
	IF_(zedConfidence < m_zedMinConfidence);

    vT.x = mT.z;	//forward
    vT.y = mT.x;	//right
    vT.z = mT.y;	//down
    vR.x = -mR.x;	//roll
    vR.y = -mR.z;	//pitch
    vR.z = -mR.y;	//yaw

    double dTot = abs(vT.x) + abs(vT.y) + abs(vT.z) + abs(vR.x) + abs(vR.y) + abs(vR.z);
    IF_(dTot < m_dMinTot);

    //make 3 channels
	Mat bChan = Mat(Size(pM1->cols, pM1->rows), pM1->type(), Scalar::all(0));
	vector<Mat> pChannels;
	pChannels.push_back(*pM1);
	pChannels.push_back(*pM2);
	pChannels.push_back(bChan);
	merge(pChannels, dM);

	//resize
	cv::resize(dM, tM, Size(m_width,m_height), 0, 0, INTER_LINEAR);
	dM = tM;

    //crop if needed
	if(m_bCrop)
	{
		Mat tM = Mat(dM, m_cropBB);
		dM = tM;
	}

	//save into list and file
	time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char strTime[128];
    strftime(strTime, sizeof(strTime), "_%F_%H-%M-%S_", tm);
    string fName = m_fNamePrefix + strTime + li2str(get_time_usec()) + m_format;

	m_ofs << fName << "\t" << vT.x << "\t" << vT.y << "\t" << vT.x << "\t" << vR.x << "\t" << vR.y << "\t" << vR.z << endl;
	imwrite(m_outDir + fName, dM);

    m_iGen++;
	LOG_I("Generated: "<< m_iGen);
}

bool _DNNGen_odometry::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->getCMat();
	IF_F(pMat->empty());

	return true;
}

}
