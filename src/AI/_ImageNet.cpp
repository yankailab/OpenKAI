/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_ImageNet.h"

namespace kai
{
_ImageNet::_ImageNet()
{
	_AIbase();

#ifdef USE_TENSORRT
	m_pIN = NULL;
#endif
	m_pRGBA = NULL;
	m_nBatch = 1;
	m_blobIn = "data";
	m_blobOut = "prob";
}

_ImageNet::~_ImageNet()
{
	DEL(m_pRGBA);
#ifdef USE_TENSORRT
	DEL(m_pIN);
#endif
}

bool _ImageNet::init(void* pKiss)
{
	CHECK_F(!this->_AIbase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("nBatch", &m_nBatch));
	F_INFO(pK->v("blobIn", &m_blobIn));
	F_INFO(pK->v("blobOut", &m_blobOut));

	m_pRGBA = new Frame();

	return true;
}

bool _ImageNet::link(void)
{
	CHECK_F(!this->_AIbase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _ImageNet::start(void)
{
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

void _ImageNet::update(void)
{
#ifdef USE_TENSORRT
	m_pIN = imageNet::Create(m_fileModel.c_str(), m_fileTrained.c_str(),
			m_fileMean.c_str(), m_fileLabel.c_str(), m_blobIn.c_str(),
			m_blobOut.c_str());
#endif

}

double _ImageNet::detect(Frame* pFrame, int* classID, string* className)
{
#ifdef USE_TENSORRT
	NULL_F(m_pIN);
#endif

	NULL_F(classID);
	NULL_F(className);
	NULL_F(pFrame);
	CHECK_F(pFrame->empty());

	m_pRGBA->getRGBAOf(pFrame);
	GpuMat* pGMat = m_pRGBA->getGMat();
	CHECK_F(pGMat->empty());
	GpuMat fGMat;
	pGMat->convertTo(fGMat, CV_32FC4);

	float prob = 0;
#ifdef USE_TENSORRT
	*classID = m_pIN->Classify((float*) fGMat.data, fGMat.cols, fGMat.rows, &prob);
	if (*classID >= 0)
	{
		*className = m_pIN->GetClassDesc(*classID);

		std::string::size_type k;
		k = className->find(',');
		if (k != std::string::npos)
			className->erase(k);
	}
#endif

	return prob;
}

}
