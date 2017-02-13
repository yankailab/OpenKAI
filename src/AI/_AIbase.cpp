/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_AIbase.h"


namespace kai
{

_AIbase::_AIbase()
{
	m_pStream = NULL;
	m_fileModel = "";
	m_fileTrained = "";
	m_fileMean = "";
	m_fileLabel = "";

	m_pObj = NULL;
	m_nObj = 16;
	m_iObj = 0;
	m_objLifetime = USEC_1SEC;

	m_sizeName = 0.5;
	m_sizeDist = 0.5;
	m_colName = Scalar(255,255,0);
	m_colDist = Scalar(0,255,255);
	m_colObs = Scalar(255,255,0);
	m_bDrawContour = false;
	m_contourBlend = 0.125;
}

_AIbase::~_AIbase()
{
	DEL(m_pObj);
}

bool _AIbase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	//Setup model
	string modelDir = "";
	string presetDir = "";

	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	F_INFO(pK->v("dir", &modelDir));

	F_INFO(pK->v("modelFile", &m_fileModel));
	F_INFO(pK->v("trainedFile", &m_fileTrained));
	F_INFO(pK->v("meanFile", &m_fileMean));
	F_INFO(pK->v("labelFile", &m_fileLabel));

	m_fileModel = modelDir + m_fileModel;
	m_fileTrained = modelDir + m_fileTrained;
	m_fileMean = modelDir + m_fileMean;
	m_fileLabel = modelDir + m_fileLabel;

	F_INFO(pK->v("objLifetime", (int*)&m_objLifetime));
	F_INFO(pK->v("sizeName", &m_sizeName));
	F_INFO(pK->v("sizeDist", &m_sizeDist));

	F_INFO(pK->v("nameB", &m_colName[0]));
	F_INFO(pK->v("nameG", &m_colName[1]));
	F_INFO(pK->v("nameR", &m_colName[2]));

	F_INFO(pK->v("distB", &m_colDist[0]));
	F_INFO(pK->v("distG", &m_colDist[1]));
	F_INFO(pK->v("distR", &m_colDist[2]));

	F_INFO(pK->v("obsB", &m_colObs[0]));
	F_INFO(pK->v("obsG", &m_colObs[1]));
	F_INFO(pK->v("obsR", &m_colObs[2]));

	F_INFO(pK->v("bDrawContour", &m_bDrawContour));
	F_INFO(pK->v("contourBlend", &m_contourBlend));

	F_INFO(pK->v("nObj", &m_nObj));
	m_pObj = new OBJECT[m_nObj];
	for (int i = 0; i < m_nObj; i++)
	{
		m_pObj[i].init();
	}
	m_iObj = 0;

	return true;
}

bool _AIbase::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("_Stream", &iName));
	m_pStream = (_StreamBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _AIbase::start(void)
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

void _AIbase::update(void)
{
	NULL_(m_pStream);
}

OBJECT* _AIbase::add(OBJECT* pNewObj)
{
	NULL_N(pNewObj);
	m_pObj[m_iObj] = *pNewObj;
	OBJECT* pNew = &m_pObj[m_iObj];

	if (++m_iObj >= m_nObj)
		m_iObj = 0;

	return pNew;
}

int _AIbase::size(void)
{
	return m_nObj;
}

OBJECT* _AIbase::get(int i, int64_t frameID)
{
	if(frameID - m_pObj[i].m_frameID >= m_objLifetime)
	{
		return NULL;
	}
	return &m_pObj[i];
}

OBJECT* _AIbase::getByClass(int iClass)
{
	int i;
	OBJECT* pObj;

	for (i = 0; i < m_nObj; i++)
	{
		pObj = &m_pObj[i];

		if (pObj->m_iClass == iClass)
			return pObj;
	}

	return NULL;
}

bool _AIbase::draw(void)
{
	IF_F(!this->_ThreadBase::draw());

	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->getCMat();
	IF_F(pMat->empty());

	Mat bg;
	if (m_bDrawContour)
	{
		bg = Mat::zeros(Size(pMat->cols, pMat->rows), CV_8UC3);
	}

	uint64_t frameID = get_time_usec() - m_objLifetime;
	for (int i = 0; i < m_nObj; i++)
	{
		OBJECT* pObj = get(i, frameID);
		if (!pObj)
			continue;
		if(pObj->m_frameID<=0)
			continue;

		Rect r;
		vInt42rect(&pObj->m_bbox, &r);

		if (pObj->m_iClass>=0)
		{
			putText(*pMat, pObj->m_name,
					Point(r.x + r.width / 2, r.y + r.height / 2),
					FONT_HERSHEY_SIMPLEX, m_sizeName, m_colName, 1);
		}

		Scalar colObs = m_colObs;
		int bolObs = 1;

		if (m_bDrawContour)
		{
			drawContours(bg, vector<vector<Point> >(1, pObj->m_contour), -1,
					colObs, CV_FILLED, 8);
		}
		else
		{
			rectangle(*pMat, r, colObs, bolObs);
		}
	}

	if (m_bDrawContour)
	{
		cv::addWeighted(*pMat, 1.0, bg, m_contourBlend, 0.0, *pMat);
	}


	return true;
}

}

