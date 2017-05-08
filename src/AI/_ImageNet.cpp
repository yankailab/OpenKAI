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
	IF_F(!this->_AIbase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	F_INFO(pK->v("maxPix", &m_maxPix));
	F_INFO(pK->v("nBatch", &m_nBatch));
	F_INFO(pK->v("blobIn", &m_blobIn));
	F_INFO(pK->v("blobOut", &m_blobOut));

	m_pRGBA = new Frame();

	string iName = "noThread";
	F_INFO(pK->v("mode", &iName));
	if (iName == "object")
		m_mode = object;

	return true;
}

bool _ImageNet::link(void)
{
	IF_F(!this->_AIbase::link());
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

		if (m_mode == object)
		{
			detect();
		}
		else
		{
			return;
		}

		this->autoFPSto();
	}
}

void _ImageNet::detect(void)
{
#ifdef USE_TENSORRT
	NULL_(m_pIN);
#endif
	NULL_(m_pStream);
	Frame* pBGR = m_pStream->bgr();
	NULL_(pBGR);
	IF_(pBGR->empty());

	m_pRGBA->getRGBAOf(pBGR);
	GpuMat gRGBA = *m_pRGBA->getGMat();
	IF_(gRGBA.empty());

	Rect bb;
	GpuMat gBB;
	GpuMat gfBB;

	for (int i = 0; i < m_iObj; i++)
	{
		OBJECT* pObj = &m_pObj[i];
		pObj->m_camSize.x = gRGBA.cols;
		pObj->m_camSize.y = gRGBA.rows;
		pObj->f2iBBox();
		IF_CONT(pObj->m_bbox.area() <= 0);
		if(pObj->m_bbox.area() > m_maxPix)
		{
			LOG_E("Image size exceeds the maxPix for ImageNet");
			continue;
		}

		if(!pObj->m_bClassify)
		{
			pObj->m_frameID = -1;
			pObj->m_iClass = -1;
			pObj->m_name = "";
			continue;
		}

		vInt42rect(&pObj->m_bbox, &bb);
		gBB = GpuMat(gRGBA, bb);
		gBB.convertTo(gfBB, CV_32FC4);

#ifdef USE_TENSORRT
		float prob = 0;
		pObj->m_iClass = m_pIN->Classify((float*) gfBB.data, gfBB.cols, gfBB.rows, &prob);
		pObj->m_prob = prob;
		pObj->m_frameID = get_time_usec();
		if (pObj->m_iClass >= 0)
			pObj->m_name = m_pIN->GetClassDesc(pObj->m_iClass);
		else
			pObj->m_name = "";
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
	IF_F(!this->_AIbase::draw());

	return true;
}

}
