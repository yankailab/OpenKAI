/*
 * _MultiImageNet.cpp
 *
 *  Created on: Oct 26, 2017
 *      Author: yankai
 */

#include "_MultiImageNet.h"

namespace kai
{

_MultiImageNet::_MultiImageNet()
{
#ifdef USE_TENSORRT
	m_pIN1 = NULL;
	m_pIN2 = NULL;
	m_pIN3 = NULL;
#endif
	m_progress = 0.0;
	m_pFrame = NULL;
}

_MultiImageNet::~_MultiImageNet()
{
}

bool _MultiImageNet::init(void* pKiss)
{
	IF_F(!this->_DataBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	m_pFrame = new Frame();

	//link
	string iName;

#ifdef USE_TENSORRT
	iName = "";
	F_INFO(pK->v("_ImageNet1", &iName));
	m_pIN1 = (_ImageNet*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_ImageNet2", &iName));
	m_pIN2 = (_ImageNet*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_ImageNet3", &iName));
	m_pIN3 = (_ImageNet*) (pK->root()->getChildInstByName(&iName));

	return true;
#else
	return false;
#endif
}

bool _MultiImageNet::start(void)
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

void _MultiImageNet::update(void)
{
	IF_(getDirFileList()<=0);

#ifdef USE_TENSORRT
	while(!m_pIN1->bReady());
	while(!m_pIN2->bReady());
	while(!m_pIN3->bReady());

	int nTot = 0;
	Mat mOut;
	string step1,step2,step3;
	m_progress = 0.0;

	for(int i=0; i<m_vFileIn.size(); i++)
	{
		string fNameIn = m_dirIn + m_vFileIn[i];
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		*m_pFrame = mIn;

		IF_CONT(m_pIN1->classify(m_pFrame, &step1)<0);
		IF_CONT(m_pIN2->classify(m_pFrame, &step2)<0);
		IF_CONT(m_pIN3->classify(m_pFrame, &step3)<0);

		string output = m_vFileIn[i] + ": Step1[" + step1 + "] Step2 [" + step2 + "] Step3 [" + step3 + "]";
		printf("%s\n",output.c_str());

		nTot++;
		double prog = (double)i/(double)m_vFileIn.size();
		if(prog - m_progress > 0.1)
		{
			m_progress = prog;
//			LOG_I("Progress: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Completed");
#endif
}

void _MultiImageNet::inference(void)
{

}


}

