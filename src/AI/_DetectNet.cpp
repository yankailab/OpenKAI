/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_DetectNet.h"

#ifdef USE_TENSORRT

namespace kai
{
_DetectNet::_DetectNet()
{
	_ThreadBase();

	num_channels_ = 0;
	m_pUniverse = NULL;
	m_pStream = NULL;
	m_pRGBA = NULL;
	m_pRGBAf = NULL;
	m_frameID = 0;
	m_confidence_threshold = 0.0;

	m_pDN = NULL;
	m_nBox = 0;
	m_nBoxMax = 0;
	m_nClass = 0;

	m_bbCPU = NULL;
	m_bbCUDA = NULL;
	m_confCPU = NULL;
	m_confCUDA = NULL;

	m_className = "target";

	modelFile = "";
	trainedFile = "";
	meanFile = "";
	labelFile = "";

}

_DetectNet::~_DetectNet()
{
}

bool _DetectNet::init(void* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss; 
	pK->m_pInst = this;

	m_pRGBA = new Frame();
	m_pRGBAf = new Frame();

	//Setup model
	string detectNetDir = "";
	string presetDir = "";

	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));

	F_INFO(pK->v("dir", &detectNetDir));
	F_FATAL_F(pK->v("modelFile", &modelFile));
	F_FATAL_F(pK->v("trainedFile", &trainedFile));
	F_FATAL_F(pK->v("meanFile", &meanFile));
	F_FATAL_F(pK->v("labelFile", &labelFile));
	F_INFO(pK->v("minConfidence", &m_confidence_threshold));
	F_INFO(pK->v("className", &m_className));

	modelFile = detectNetDir + modelFile;
	trainedFile = detectNetDir + trainedFile;
	meanFile = detectNetDir + meanFile;
	labelFile = presetDir + labelFile;

	return true;
}

bool _DetectNet::link(void)
{
	NULL_F(m_pKiss);
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";
	F_ERROR_F(pK->v("_Stream", &iName));
	m_pStream = (_Stream*) (pK->root()->getChildInstByName(&iName));
	F_ERROR_F(pK->v("_Universe", &iName));
	m_pUniverse = (_Universe*) (pK->root()->getChildInstByName(&iName));

	m_pUniverse->addObjClass(&m_className, 0);

	return true;
}

bool _DetectNet::start(void)
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

void _DetectNet::update(void)
{

	m_pDN = detectNet::Create(modelFile.c_str(), trainedFile.c_str(),
			meanFile.c_str(), m_confidence_threshold);
	NULL_(m_pDN);

	m_nBoxMax = m_pDN->GetMaxBoundingBoxes();
	m_nClass = m_pDN->GetNumClasses();

	CHECK_(
			!cudaAllocMapped((void** )&m_bbCPU, (void** )&m_bbCUDA,
					m_nBoxMax * sizeof(float4)));
	CHECK_(
			!cudaAllocMapped((void** )&m_confCPU, (void** )&m_confCUDA,
					m_nBoxMax * m_nClass * sizeof(float)));

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		m_frameID = get_time_usec();

		detectFrame();

		this->autoFPSto();
	}

}

void _DetectNet::detectFrame(void)
{
	NULL_(m_pStream);
	NULL_(m_pUniverse);
	NULL_(m_pDN);

	Frame* pBGR = m_pStream->getBGRFrame();
	NULL_(pBGR);
	CHECK_(pBGR->empty());
	CHECK_(m_pRGBA->isNewerThan(pBGR));

	m_pRGBA->getRGBAOf(pBGR);
	GpuMat* pGMat = m_pRGBA->getGMat();
	CHECK_(pGMat->empty());

	GpuMat fGMat;
	pGMat->convertTo(fGMat, CV_32FC4);

	/*	m_pRGBA->getRGBAOf(pBGR);
	 m_pRGBAf->get32FC4Of(m_pRGBA);
	 GpuMat* fGMat = m_pRGBA->getGMat();
	 */
	m_nBox = m_nBoxMax;

	CHECK_(!m_pDN->Detect((float* )fGMat.data, fGMat.cols, fGMat.rows, m_bbCPU,
					&m_nBox, m_confCPU));

	LOG(INFO)<<m_nBox<<" bboxes detected";

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

		m_pUniverse->addObject(&obj);
	}

}

bool _DetectNet::draw(Frame* pFrame, vInt4* pTextPos)
{
	NULL_F(pFrame);

	putText(*pFrame->getCMat(), "DetectNet FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);
	pTextPos->m_y += pTextPos->m_w;

	return true;
}

}

#endif
