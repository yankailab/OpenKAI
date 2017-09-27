/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_ImageNet.h"

namespace kai
{

_ImageNet::_ImageNet()
{
#ifdef USE_TENSORRT
	m_pIN = NULL;
#endif

	m_mode = noThread;
	m_pRGBA = NULL;
	m_nBatch = 1;
	m_blobIn = "data";
	m_blobOut = "prob";

	m_maxPix = 100000;
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
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	KISSm(pK,maxPix);
	KISSm(pK,nBatch);
	KISSm(pK,blobIn);
	KISSm(pK,blobOut);

	m_pRGBA = new Frame();

	string iName = "noThread";
	F_INFO(pK->v("mode", &iName));
	if (iName == "thread")
	{
		m_mode = thread;
		bSetActive(true);
	}

	return true;
}

bool _ImageNet::link(void)
{
	IF_F(!this->_DetectorBase::link());
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
	m_pIN = imageNet::Create(m_modelFile.c_str(), m_trainedFile.c_str(),
			m_meanFile.c_str(), m_labelFile.c_str(), m_blobIn.c_str(),
			m_blobOut.c_str());

	NULL_(m_pIN);
#endif

	IF_(m_mode == noThread);

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _ImageNet::detect(void)
{
#ifdef USE_TENSORRT
	NULL_(m_pIN);
#endif

	IF_(!m_bActive);
	NULL_(m_pVision);
	Frame* pBGR = m_pVision->bgr();
	NULL_(pBGR);
	IF_(pBGR->empty());

	m_pRGBA->getRGBAOf(pBGR);
	GpuMat gRGBA = *m_pRGBA->getGMat();
	IF_(gRGBA.empty());

	if(m_pDetIn)
	{
		m_obj = m_pDetIn->m_obj;
	}

	Rect bb;
	GpuMat gBB;
	GpuMat gfBB;

	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		pO->m_camSize.x = gRGBA.cols;
		pO->m_camSize.y = gRGBA.rows;
		pO->f2iBBox();
		IF_CONT(pO->m_bbox.area() <= 0);
		if(pO->m_bbox.area() > m_maxPix)
		{
			LOG_E("Image size exceeds the maxPix for ImageNet");
			continue;
		}

		vInt42rect(&pO->m_bbox, &bb);
		gBB = GpuMat(gRGBA, bb);
		gBB.convertTo(gfBB, CV_32FC4);

#ifdef USE_TENSORRT
		float prob = 0;
		int iClass = m_pIN->Classify((float*) gfBB.data, gfBB.cols, gfBB.rows, &prob);
		if(prob < m_minConfidence)
		{
			pO->m_iClass = -1;
			pO->m_name = "";
			pO->m_prob = 0.0;
		}
		else
		{
			pO->m_iClass = iClass;
			pO->m_name = m_pIN->GetClassDesc(pO->m_iClass);
			pO->m_prob = prob;
		}
		pO->m_frameID = get_time_usec();
#endif
	}
}

int _ImageNet::classify(Frame* pImg, string* pName)
{
	if (!pImg)
		return -1;
	GpuMat* gM = pImg->getGMat();
	if (!gM)
		return -1;
	if (gM->empty())
		return -1;

	GpuMat gfM;
	gM->convertTo(gfM, CV_32FC3);

	int iClass = -1;
#ifdef USE_TENSORRT
	if (!m_pIN)
		return -1;
	float prob = 0;
	iClass = m_pIN->Classify((float*) gfM.data, gfM.cols, gfM.rows, &prob);

	if (iClass < 0)
		return -1;

	if (pName)
	{
		*pName = m_pIN->GetClassDesc(iClass);

		std::string::size_type k;
		k = pName->find(',');
		if (k != std::string::npos)
			pName->erase(k);
	}
#endif

	return iClass;
}

bool _ImageNet::draw(void)
{
	IF_F(!this->_DetectorBase::draw());

	return true;
}

}
