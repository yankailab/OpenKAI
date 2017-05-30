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
	m_pFlow = NULL;
	m_pZED = NULL;
	m_nGen = 0;
	m_iGen = 0;
	m_dMinTot = 0;
	m_outDir = "";

	m_zedMinConfidence = 0;
	m_uDelay = 0;

	m_pFrameA = NULL;
	m_pFrameB = NULL;
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
	KISSm(pK,nGen);
	KISSm(pK,uDelay);
	KISSm(pK,zedMinConfidence);
	KISSm(pK,dMinTot);
	KISSm(pK,bCrop);
	if (m_bCrop != 0)
	{
		F_INFO(pK->v("cropX", &m_cropBB.x));
		F_INFO(pK->v("cropY", &m_cropBB.y));
		F_INFO(pK->v("cropW", &m_cropBB.width));
		F_INFO(pK->v("cropH", &m_cropBB.height));
	}

	string outFile = m_outDir + "dnnOdomTrainList.txt";
	m_ofs.open(outFile.c_str(), ios::out);

	m_pFrameA = new Frame();
	m_pFrameB = new Frame();

	return true;
}

bool _DNNGen_odometry::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("_Flow", &iName));
	m_pFlow = (_Flow*) (pK->root()->getChildInstByName(&iName));

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

		if(m_iGen >= m_nGen)
		{
			m_ofs.close();
			exit(0);
		}

		sample();

		this->autoFPSto();
	}
}

void _DNNGen_odometry::sample(void)
{
	NULL_(m_pZED);
	IF_(!m_pZED->isOpened());
	NULL_(m_pFlow);

	Mat flowImg;
	stringstream ss;
	vDouble3 vT,vR,mT,mR;
	uint64_t dT;

	//initial shot
	m_pFrameA->update(m_pZED->gray());
	m_pZED->getMotionDelta(&mT, &mR, &dT);

	//insert certain delay
	if(m_uDelay > 0)
		usleep(m_uDelay);

	//second shot
	m_pFrameB->update(m_pZED->gray());
	int zedConfidence = m_pZED->getMotionDelta(&mT, &mR, &dT);

    //get opt flow
	IF_(!m_pFlow->addFrame(false,m_pFrameA));
	IF_(!m_pFlow->addFrame(true,m_pFrameB));

	//check
	IF_(zedConfidence < m_zedMinConfidence);

    vT.x = mT.z;	//forward
    vT.y = mT.x;	//right
    vT.z = mT.y;	//down
    vR.x = -mR.x;	//roll
    vR.y = -mR.z;	//pitch
    vR.z = -mR.y;	//yaw

    double dTot = abs(vT.x) + abs(vT.y) + abs(vT.z) + abs(vR.x) + abs(vR.y) + abs(vR.z);
    IF_(dTot < m_dMinTot);

    //crop if needed
	if(m_bCrop)
	{
		Mat fMat;
		m_pFlow->flowMat()->download(fMat);
		flowImg = Mat(fMat, m_cropBB);
	}
	else
	{
		m_pFlow->flowMat()->download(flowImg);
	}

	//make 3 channels
	vector<Mat> pChannels;
	split(flowImg, pChannels);
	Mat bChan = Mat(Size(pChannels[0].cols, pChannels[0].rows), pChannels[0].type(), Scalar::all(0));
	pChannels.push_back(bChan);
	merge(pChannels, flowImg);

	ss << setfill('0') << setw(10) << right << m_iGen;
	m_ofs << ss.str() << ".png" << "\t" << vT.x << "\t" << vT.y << "\t" << vT.x << "\t" << vR.x << "\t" << vR.y << "\t" << vR.z << endl;

	imwrite(m_outDir + ss.str() + ".png", flowImg);
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
