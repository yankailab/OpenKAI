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
	this->~_AIbase();
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
	NULL_(m_pIN);
#endif

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}

}

void _ImageNet::detect(void)
{
	NULL_(m_pStream);
#ifdef USE_TENSORRT
	NULL_(m_pIN);
#endif

	m_pObj = m_pStream->getObject();
	NULL_(m_pObj);
	CHECK_(m_pObj->size()<=0);

	Frame* pBGR = m_pStream->bgr();
	NULL_(pBGR);
	CHECK_(pBGR->empty());

	m_pRGBA->getRGBAOf(pBGR);
	GpuMat* pGMat = m_pRGBA->getGMat();
	CHECK_(pGMat->empty());
	GpuMat fGMat;
	pGMat->convertTo(fGMat, CV_32FC4);

	uint64_t frameID = get_time_usec()-m_dTime;

	for (int i = 0; i < m_pObj->size(); i++)
	{
		OBJECT* pObj = m_pObj->get(i,frameID);
		if(!pObj)continue;
		Rect r;
		vInt42rect(&pObj->m_bbox, &r);
		GpuMat oGMat = GpuMat(fGMat,r);

#ifdef USE_TENSORRT
		pObj->m_iClass = m_pIN->Classify((float*) oGMat.data, oGMat.cols, oGMat.rows, &pObj->m_prob);
		if (pObj->m_iClass >= 0)
		{
			pObj->m_name = m_pIN->GetClassDesc(pObj->m_iClass);
		}
#endif
	}

}

bool _ImageNet::draw(void)
{
	CHECK_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	return true;
}

}
