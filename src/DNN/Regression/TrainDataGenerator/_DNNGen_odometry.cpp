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
	m_outDir = "";
	m_width = 224;
	m_height = 224;

	m_zedMinConfidence = 0.0;
	m_dTfrom = 0.0;
	m_dTto = USEC_1SEC;

	m_pFrameA = NULL;
	m_pFrameB = NULL;
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
			return;
		}

		sample();

		LOG_I("Generated: "<< m_iGen);

		this->autoFPSto();
	}
}

void _DNNGen_odometry::sample(void)
{
	NULL_(m_pZED);
	NULL_(m_pFlow);
	IF_(!m_pZED->isOpened());

	Mat			flowImg;
	vDouble3	vT,vR;
	stringstream ss;
	vT.init();
	vR.init();
	vDouble3 mT,mR;
	uint64_t dT;
	int zedConfidence;

	flowImg = Mat(Size(m_width, m_height), CV_8UC3, Scalar::all(0));

	//initialize
	zedConfidence = m_pZED->getMotionDelta(&mT, &mR, &dT);

	//get initial rgb frame
	m_pFrameA->update(m_pZED->gray());

	//get ZED odometry
	zedConfidence = m_pZED->getMotionDelta(&mT, &mR, &dT);
	IF_(zedConfidence < 0);
	IF_(dT <= 0);
	IF_(dT > USEC_10SEC);

    vT.x = mT.z;	//forward
    vT.y = mT.x;	//right
    vT.z = mT.y;	//down
    vR.x = -mR.x;  //roll
    vR.y = -mR.z;  //pitch
    vR.z = -mR.y;  //yaw

	//get the second rgb frame and optical flow
	m_pFrameB->update(m_pZED->gray());

	m_pFlow->addFrame(false,m_pFrameA);
	m_pFlow->addFrame(true,m_pFrameB);

	Mat fMat;
	m_pFlow->flowMat()->download(fMat);

	//TODO:cutout the center part of the flow


	ss << setfill('0') << setw(10) << right << m_iGen;
	m_ofs << ss.str() << ".png" << "\t" << vT.x << "\t" << vT.y << "\t" << vT.x << "\t" << vR.x << "\t" << vR.y << "\t" << vR.z << endl;

	imwrite(m_outDir + ss.str() + ".png", flowImg);

}

bool _DNNGen_odometry::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->getCMat();
	IF_F(pMat->empty());

	return true;
}

}
