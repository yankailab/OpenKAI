/*
 * ObjectLocalizer.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "_FastDetector.h"

namespace kai
{
_FastDetector::_FastDetector()
{
	_ThreadBase();
	DetectorBase();

	m_bThreadON = false;
	m_threadID = 0;

	m_iHuman = 0;
	m_numHuman = 0;
	m_pHuman = NULL;
	m_frameID = 0;
	m_objLifeTime = 100000;

	scale = 1.05;
	nlevels = 13;
	gr_threshold = 8;
	hit_threshold = 1.4;

	win_width = 48;
	win_stride_width = 8;
	win_stride_height = 8;
	block_width = 16;
	block_stride_width = 8;
	block_stride_height = 8;
	cell_width = 8;
	nbins = 9;
}

_FastDetector::~_FastDetector()
{
}

bool _FastDetector::init(JSON* pJson)
{
	string cascadeFile;
	CHECK_ERROR(pJson->getVal("CASCADE_FILE", &cascadeFile));
	m_pCascade = cuda::CascadeClassifier::create(cascadeFile);

	//HOG for Pedestrian detection
	string hogFile;
	CHECK_ERROR(pJson->getVal("HOG_FILE", &hogFile));

	Size win_stride(win_stride_width, win_stride_height);
	Size win_size(win_width, win_width * 2);
	Size block_size(block_width, block_width);
	Size block_stride(block_stride_width, block_stride_height);
	Size cell_size(cell_width, cell_width);

	m_pHumanHOG = cuda::HOG::create(win_size, block_size, block_stride, cell_size, nbins);
	m_pHumanHOG->setSVMDetector(m_pHumanHOG->getDefaultPeopleDetector());

	CHECK_INFO(pJson->getVal("CASCADE_HUMAN_LIFETIME", &m_objLifeTime));
	CHECK_ERROR(pJson->getVal("CASCADE_HUMAN_NUM", &m_numHuman));
	m_pHuman = new FAST_OBJECT[m_numHuman];
	if(m_pHuman==NULL)return false;
	m_iHuman = 0;

	int i;
	for(i=0; i<m_numHuman; i++)
	{
		m_pHuman[i].m_status = OBJ_VACANT;
	}


	m_pBGRA = new CamFrame();

	return true;
}

bool _FastDetector::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode << " in FastDetector::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO)<< "FastDetector.start()";

	return true;
}

void _FastDetector::update(void)
{
	m_tSleep = TRD_INTERVAL_FASTDETECTOR;

	while (m_bThreadON)
	{
		this->updateTime();

		m_frameID = this->m_timeStamp;
		deleteOutdated();
		detect();

		//sleepThread can be woke up by this->wakeupThread()
		if(m_tSleep > 0)
		{
			this->sleepThread(0, m_tSleep);
		}
	}

}

void _FastDetector::updateFrame(CamFrame* pFrame, CamFrame* pGray)
{
	if(pFrame==NULL)return;
	if(pFrame->empty())return;
	if(pGray==NULL)return;
	if(pGray->empty())return;

	pFrame->getBGRA(m_pBGRA);
	m_pGray->updateFrame(pGray);

	this->wakeupThread();
}

inline int _FastDetector::findVacancy(int iStart)
{
	FAST_OBJECT* pObj;
	int iComplete = iStart - 1;

	if(iStart >= m_numHuman)
	{
		iStart = 0;
		iComplete = m_numHuman-1;
	}

	do
	{
		pObj = &m_pHuman[iStart];
		if(pObj->m_status == OBJ_VACANT)
		{
			return iStart;
		}

		if(++iStart >= m_numHuman)iStart = 0;
	}
	while(iStart != iComplete);

	return -1;
}

inline void _FastDetector::deleteOutdated(void)
{
	int i;
	FAST_OBJECT* pObj;

	for(i=0;i<m_numHuman;i++)
	{
		pObj = &m_pHuman[i];
		if(m_frameID - pObj->m_frameID > m_objLifeTime)
		{
			pObj->m_status = OBJ_VACANT;
		}
	}
}

void _FastDetector::detect(void)
{
	int i,iVacant;
	FAST_OBJECT* pObj;
	GpuMat cascadeGMat;
	vector<Rect> vRect;

	if(m_pGray->empty())return;

	iVacant = 0;

	if (m_pCascade)
	{
		//	m_pCascade->setFindLargestObject(false);
		m_pCascade->setScaleFactor(1.2);
		//	m_pCascade->setMinNeighbors((filterRects || findLargestObject) ? 4 : 0);

		m_pCascade->detectMultiScale(*m_pGray->getCurrentFrame(), cascadeGMat);
		m_pCascade->convert(cascadeGMat, vRect);

		for (i = 0; i < vRect.size(); i++)
		{
			iVacant = findVacancy(iVacant+1);

			//No vacant space
			if(iVacant<0)return;

			pObj = &m_pHuman[iVacant];
			pObj->m_boundBox = vRect[i];
			pObj->m_frameID = m_frameID;
			pObj->m_status = OBJ_ADDED;
		}
	}

//	vRect.clear();
//	Size win_stride(win_stride_width, win_stride_height);
//
//	m_pHumanHOG->setNumLevels(nlevels);
//	m_pHumanHOG->setHitThreshold(hit_threshold);
//	m_pHumanHOG->setWinStride(win_stride);
//	m_pHumanHOG->setScaleFactor(scale);
//	m_pHumanHOG->setGroupThreshold(gr_threshold);
//	m_pHumanHOG->detectMultiScale(*m_pGray->getCurrentFrame(), vRect);
//
//	//	m_numHuman = 0;
//	for (i = 0; i < vRect.size(); i++)
//	{
//		m_pHuman[m_numHuman].m_boundBox = vRect[i];
//		m_numHuman++;
//		if (m_numHuman == NUM_FASTOBJ)
//		{
//			break;
//		}
//	}

}

int _FastDetector::getHuman(FAST_OBJECT** ppHuman)
{
	*ppHuman = m_pHuman;
	return m_numHuman;
}

}

