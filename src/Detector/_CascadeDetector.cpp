/*
 * ObjectLocalizer.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "_CascadeDetector.h"

namespace kai
{

_CascadeDetector::_CascadeDetector()
{
	_ThreadBase();
	DetectorBase();

	m_bThreadON = false;
	m_threadID = 0;

	m_iObj = 0;
	m_numObj = 0;
	m_pObj = NULL;
	m_frameID = 0;
	m_objLifeTime = 100000;
	m_posDiff = 0;

}

_CascadeDetector::~_CascadeDetector()
{
}

bool _CascadeDetector::init(string name, JSON* pJson)
{
	string cascadeFile;
	CHECK_ERROR(pJson->getVal("CASCADE_FILE_"+name, &cascadeFile));
	m_pCascade = cuda::CascadeClassifier::create(cascadeFile);
	//TODO:set the upper limit of objects to be detected
	//m_pCascade->

	CHECK_INFO(pJson->getVal("CASCADE_LIFETIME_"+name, &m_objLifeTime));
	CHECK_INFO(pJson->getVal("CASCADE_POSDIFF_"+name, &m_posDiff));
	CHECK_ERROR(pJson->getVal("CASCADE_NUM_"+name, &m_numObj));
	m_pObj = new CASCADE_OBJECT[m_numObj];
	if(m_pObj==NULL)return false;
	m_iObj = 0;

	int i;
	for(i=0; i<m_numObj; i++)
	{
		m_pObj[i].m_status = OBJ_VACANT;
	}

	return true;
}

bool _CascadeDetector::start(void)
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

void _CascadeDetector::update(void)
{
	m_tSleep = TRD_INTERVAL_CASCADEDETECTOR;

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

void _CascadeDetector::detect(void)
{
	int i,j,iVacant;
	CASCADE_OBJECT* pObj;
	GpuMat cascadeGMat;
	vector<Rect> vRect;
	Rect iRect;
	bool bNewObj;

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
			iRect = vRect[i];
			bNewObj = true;

			if(m_posDiff)
			{
				//Find the correspondence to existing object
				for(j=0;j<m_numObj;j++)
				{
					pObj = &m_pObj[j];
					if(pObj->m_status==OBJ_VACANT)continue;

					if(abs(pObj->m_boundBox.x-iRect.x)<=m_posDiff &&
							abs(pObj->m_boundBox.y-iRect.y)<=m_posDiff &&
							abs(pObj->m_boundBox.width-iRect.width)<=m_posDiff &&
							abs(pObj->m_boundBox.height-iRect.height)<=m_posDiff)
					{
						pObj->m_boundBox = iRect;
						pObj->m_frameID = m_frameID;
						bNewObj = false;
						break;
					}
				}

				if(!bNewObj)continue;
			}

			//Newly detected, allocate new space to hold it
			iVacant = findVacancy(iVacant+1);

			//No vacant space
			if(iVacant<0)return;

			pObj = &m_pObj[iVacant];
			pObj->m_boundBox = vRect[i];
			pObj->m_frameID = m_frameID;
			pObj->m_status = OBJ_ADDED;
		}
	}

}

void _CascadeDetector::updateFrame(CamFrame* pFrame, CamFrame* pGray)
{
	if(pFrame==NULL)return;
	if(pFrame->empty())return;
	if(pGray==NULL)return;
	if(pGray->empty())return;

//	pFrame->getBGRA(m_pBGRA);
	m_pGray->updateFrame(pGray);

	this->wakeupThread();
}

inline int _CascadeDetector::findVacancy(int iStart)
{
	CASCADE_OBJECT* pObj;
	int iComplete = iStart - 1;

	if(iStart >= m_numObj)
	{
		iStart = 0;
		iComplete = m_numObj-1;
	}

	do
	{
		pObj = &m_pObj[iStart];
		if(pObj->m_status == OBJ_VACANT)
		{
			return iStart;
		}

		if(++iStart >= m_numObj)iStart = 0;
	}
	while(iStart != iComplete);

	return -1;
}

inline void _CascadeDetector::deleteOutdated(void)
{
	int i;
	CASCADE_OBJECT* pObj;

	for(i=0;i<m_numObj;i++)
	{
		pObj = &m_pObj[i];
		if(m_frameID - pObj->m_frameID > m_objLifeTime)
		{
			pObj->m_status = OBJ_VACANT;
		}
	}
}


int _CascadeDetector::getObjList(CASCADE_OBJECT** ppObj)
{
	*ppObj = m_pObj;
	return m_numObj;
}

}

