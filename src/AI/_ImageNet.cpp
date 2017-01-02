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
	m_detectDist = 0.0;
	m_detectMinSize = 0.0;
	m_bDrawContour = false;
	m_contourBlend = 0.125;
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
	F_INFO(pK->v("detectDist", &m_detectDist));
	F_INFO(pK->v("detectMinSize", &m_detectMinSize));
	F_INFO(pK->v("bDrawContour", &m_bDrawContour));
	F_INFO(pK->v("contourBlend", &m_contourBlend));

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

	m_pObj->reset();
	CHECK_(m_pStream->findObjects(NULL, m_pObj, m_detectDist, m_detectMinSize)<=0);

	Frame* pBGR = m_pStream->bgr();
	NULL_(pBGR);
	CHECK_(pBGR->empty());

	m_pRGBA->getRGBAOf(pBGR);
	GpuMat* pGMat = m_pRGBA->getGMat();
	CHECK_(pGMat->empty());
	GpuMat fGMat;
	pGMat->convertTo(fGMat, CV_32FC4);

	OBJECT* pObj;
	for (int i = 0; i < m_pObj->size(); i++)
	{
		pObj = m_pObj->get(i);
		if(!pObj)break;
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
	CHECK_F(pMat->empty());

	Mat bg;
	if(m_bDrawContour)
	{
		bg = Mat::zeros(Size(pMat->cols,pMat->rows), CV_8UC3);
	}

	OBJECT* pObj;
	int i = 0;
	while ((pObj = m_pObj->get(i++)))
	{
		Rect r;
		vInt42rect(&pObj->m_bbox, &r);
		putText(*pMat, pObj->m_name,
				Point(r.x + r.width / 2, r.y + r.height / 2),
				FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 1);

		if(m_bDrawContour)
		{
			drawContours(bg, vector<vector<Point> >(1,pObj->m_contour), -1, Scalar(0, 255, 0), CV_FILLED, 8);
		}
		else
		{
			rectangle(*pMat, r, Scalar(0, 255, 0), 1);
		}
	}

	if(m_bDrawContour)
	{
		cv::addWeighted( *pMat, 1.0, bg, m_contourBlend, 0.0, *pMat);
	}

	return true;
}

}
