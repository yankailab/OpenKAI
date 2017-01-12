/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_DetectNet.h"

namespace kai
{
_DetectNet::_DetectNet()
{
	_AIbase();

	num_channels_ = 0;
	m_pRGBA = NULL;
	m_pRGBAf = NULL;
	m_minCofidence = 0.0;

#ifdef USE_TENSORRT
	m_pDN = NULL;
#endif
	m_nBox = 0;
	m_nBoxMax = 0;
	m_nClass = 0;

	m_bbCPU = NULL;
	m_bbCUDA = NULL;
	m_confCPU = NULL;
	m_confCUDA = NULL;

	m_className = "";
}

_DetectNet::~_DetectNet()
{
	this->~_AIbase();
}

bool _DetectNet::init(void* pKiss)
{
	CHECK_F(!this->_AIbase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("minConfidence", &m_minCofidence));
	F_INFO(pK->v("className", &m_className));

	m_pRGBA = new Frame();
	m_pRGBAf = new Frame();

	return true;
}

bool _DetectNet::link(void)
{
	CHECK_F(!this->_AIbase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _DetectNet::start(void)
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

void _DetectNet::update(void)
{
#ifdef USE_TENSORRT
	m_pDN = detectNet::Create(m_fileModel.c_str(), m_fileTrained.c_str(),
			m_fileMean.c_str(), m_minCofidence);
	NULL_(m_pDN);

	m_nBoxMax = m_pDN->GetMaxBoundingBoxes();
	m_nClass = m_pDN->GetNumClasses();

	CHECK_(	!cudaAllocMapped((void** )&m_bbCPU, (void** )&m_bbCUDA,
					m_nBoxMax * sizeof(float4)));
	CHECK_(	!cudaAllocMapped((void** )&m_confCPU, (void** )&m_confCUDA,
					m_nBoxMax * m_nClass * sizeof(float)));
#endif

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}

}

void _DetectNet::detect(void)
{
	NULL_(m_pStream);
#ifdef USE_TENSORRT
	NULL_(m_pDN);
#endif

	Frame* pBGR = m_pStream->bgr();
	NULL_(pBGR);
	CHECK_(pBGR->empty());
	CHECK_(m_pRGBA->isNewerThan(pBGR));

	m_pRGBA->getRGBAOf(pBGR);
	GpuMat* pGMat = m_pRGBA->getGMat();
	CHECK_(pGMat->empty());

	GpuMat fGMat;
	pGMat->convertTo(fGMat, CV_32FC4);

	m_nBox = m_nBoxMax;

#ifdef USE_TENSORRT
	CHECK_(
			!m_pDN->Detect((float* )fGMat.data, fGMat.cols, fGMat.rows, m_bbCPU,
					&m_nBox, m_confCPU));

	LOG_I("Detected BBox: "<<m_nBox);

	OBJECT obj;
	for (int n = 0; n < m_nBox; n++)
	{
		const int nc = m_confCPU[n * 2 + 1];
		float* bb = m_bbCPU + (n * 4);

		obj.m_iClass = 0;
		obj.m_bbox.m_x = (int) bb[0];
		obj.m_bbox.m_y = (int) bb[1];
		obj.m_bbox.m_z = (int) bb[2];
		obj.m_bbox.m_w = (int) bb[3];
		obj.m_camSize.m_x = fGMat.cols;
		obj.m_camSize.m_y = fGMat.rows;
		obj.m_dist = 0.0;
		obj.m_prob = 0.0;
		obj.m_name = m_className;

		add(&obj);
	}
#endif

}

bool _DetectNet::draw(void)
{
	CHECK_F(!this->_AIbase::draw());

	return true;
}

}
