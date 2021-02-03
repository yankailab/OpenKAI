/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_ImageNet.h"

#ifdef USE_OPENCV
#ifdef USE_JETSON_INFERENCE
/*
namespace kai
{

_ImageNet::_ImageNet()
{
	m_pIN = NULL;
	m_pRGBA = NULL;
	m_nBatch = 1;
	m_blobIn = "data";
	m_blobOut = "prob";
	m_maxPix = 100000;
	m_pmClass = NULL;
	m_piClass = NULL;
}

_ImageNet::~_ImageNet()
{
	DEL(m_pRGBA);
	DEL(m_pIN);
	DEL(m_pmClass);
	DEL(m_piClass);
}

bool _ImageNet::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,maxPix);
	KISSm(pK,nBatch);
	KISSm(pK,blobIn);
	KISSm(pK,blobOut);

	m_pRGBA = new Frame();
	m_pmClass = new uint64_t[m_nBatch];
	m_piClass = new int[m_nBatch];

	return true;
}

bool _ImageNet::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdate, this);
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
	m_pIN = imageNet::Create(m_fModel.c_str(),
							 m_fWeight.c_str(),
							 m_fMean.c_str(),
							 m_labelFile.c_str(),
							 m_blobIn.c_str(),
							 m_blobOut.c_str(),
							 m_nBatch);
	NULL_(m_pIN);

	m_nClass = m_pIN->GetNumClasses();
	for(int i=0; i<m_nClass; i++)
	{
		m_pClassStatis[i].m_name = m_pIN->GetClassDesc(i);
	}

	m_bReady = true;

	IF_(m_mode == noThread);

	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		detect();

		m_pT->autoFPSto();
	}
}

int _ImageNet::getClassIdx(string& className)
{
	if(!bReady())return -1;

	int i;
	for(i=0; i<m_nClass; i++)
	{
		if(m_pIN->GetClassDesc(i) == className)
			return i;
	}

	return -1;
}

string _ImageNet::getClassName(int iClass)
{
	string className = m_pIN->GetClassDesc(iClass);
	return className;
}

void _ImageNet::detect(void)
{
	IF_(!bReady());
	IF_(!m_bActive);

	NULL_(m_pVision);
	Frame* pBGR = m_pVision->BGR();
	NULL_(pBGR);
	IF_(pBGR->bEmpty());

	GpuMat gfRGBA;
	*m_pRGBA = pBGR->rgba();
	m_pRGBA->getGMat()->convertTo(gfRGBA, CV_32FC4);

	if(m_pDetIn)
	{
		m_obj = m_pDetIn->m_obj;
	}

	Rect bb;
	GpuMat gfBB;
	int nBatch = 0;
	int iBatchFrom = 0;
	for(int i=0; i<m_obj.size(); i++)
	{
		OBJECT* pO = m_obj.at(i);
		pO->m_camSize.x = gfRGBA.cols;
		pO->m_camSize.y = gfRGBA.rows;
		pO->f2iBBox();
		IF_CONT(pO->m_bbox.area() <= 0);
		if(pO->m_bbox.area() > m_maxPix)
		{
			LOG_E("Image size exceeds the maxPix for ImageNet");
			return;
		}

		vInt42rect(&pO->m_bbox, &bb);
		gfBB = GpuMat(gfRGBA, bb);

		m_pIN->AddImgToBatch(nBatch++, (float*) gfBB.data, gfBB.cols, gfBB.rows);
		if(nBatch == m_nBatch)
		{
			classifyBatch(iBatchFrom, nBatch);
			nBatch = 0;
			iBatchFrom = i+1;
		}
	}
	if(nBatch>0)
	{
		classifyBatch(iBatchFrom, nBatch);
	}
}

void _ImageNet::classifyBatch(int iBatchFrom, int nBatch)
{
	m_pIN->ClassifyBatch(nBatch, m_pmClass, m_piClass, (float)m_minConfidence);
	uint64_t tStamp = getTimeUsec();

	for(int i=0; i<nBatch; i++)
	{
		OBJECT* pO = m_obj.at(i+iBatchFrom);

		pO->resetClass();
		pO->m_mClass = m_pmClass[i];
		pO->m_iClass = m_piClass[i];
		pO->m_tStamp = tStamp;
	}
}

int _ImageNet::classify(Frame* pBGR, string* pName)
{
	if(!m_pIN)return -1;
	if(!pBGR)return -1;
	if(pBGR->bEmpty())return -1;

	*m_pRGBA = pBGR->rgba();
	GpuMat gRGBA = *m_pRGBA->getGMat();
	if(gRGBA.empty())return -1;

	GpuMat gfM;
	gRGBA.convertTo(gfM, CV_32FC4);

	int iClass = -1;
	float prob = 0;
	iClass = m_pIN->Classify((float*) gfM.data, gfM.cols, gfM.rows, &prob);

	if (iClass < 0)return -1;

	if (pName)
	{
		*pName = m_pIN->GetClassDesc(iClass);

		std::string::size_type k;
		k = pName->find(',');
		if (k != std::string::npos)
			pName->erase(k);
	}

	return iClass;
}

bool _ImageNet::draw(void)
{
	IF_F(!this->_DetectorBase::draw());

	return true;
}

}
*/
#endif
#endif

