/*
 * _Universe.cpp
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#include "_Universe.h"

namespace kai
{

_Universe::_Universe()
{
	_ThreadBase();

	m_numObj = NUM_OBJ;
	m_numBatch = 0;
	m_frameID = 0;
	m_frameLifeTime = 0;
	m_objProbMin = 0;
	m_disparity = 0;

	reset();

}

_Universe::~_Universe()
{
	// TODO Auto-generated destructor stub
}

bool _Universe::init(Config* pConfig)
{
	if (this->_ThreadBase::init(pConfig)==false)
		return false;

//	if(m_caffe.init(pJson,"")==false)return false;

	F_ERROR_F(pConfig->v("frameLifetime", &m_frameLifeTime));
	F_ERROR_F(pConfig->v("probMin", &m_objProbMin));
	F_ERROR_F(pConfig->v("posDisparity", &m_disparity));

	return true;
}

bool _Universe::start(void)
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

void _Universe::update(void)
{
	m_caffe.setModeGPU();

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		m_frameID = get_time_usec();

		classifyObject();

		this->autoFPSto();
	}

}

void _Universe::deleteObject(int i)
{
	OBJECT* pObj = &m_pObjects[i];

	//Follow the order, change the status after releasing m_pMat
	pObj->m_vContours.clear();
	pObj->m_status = OBJ_VACANT;
}

void _Universe::classifyObject(void)
{
	int i, j;
	OBJECT* pObj;
	int numBatch;
	OBJECT* pObjBatch[NUM_DETECT_BATCH];
	string* pName;

	numBatch = 0;

	//Collect the candidates
	for (i = 0; i < m_numObj; i++)
	{
		pObj = &m_pObjects[i];

		//Delete the outdated frame
		if (m_frameID - pObj->m_frameID > m_frameLifeTime)
		{
			deleteObject(i);
			continue;
		}

		//Delete the uncertain ones
//		if(pObj->m_status == OBJ_COMPLETE)
//		{
//			if(pObj->m_prob[0]<=m_objProbMin)
//			{
//				deleteObject(i);
//				continue;
//			}
//		}
//
//		if(pObj->m_status != OBJ_ADDED)continue;
//		if(pObj->m_Mat.empty())continue;
//
//		//TODO: resize
//		pObj->m_status = OBJ_CLASSIFYING;
//		m_vMat.push_back(pObj->m_Mat);
//
//		pObjBatch[numBatch] = pObj;
//
//		numBatch++;
//		if(numBatch >= NUM_DETECT_BATCH)break;
	}

	if (numBatch <= 0)
		return;

	return;

	//Get the top 5 possible labels
	m_vPredictions = m_caffe.ClassifyBatch(m_vMat, 5);
	m_vMat.clear();

	for (i = 0; i < numBatch; i++)
	{
		pObj = pObjBatch[i];

		for (j = 0; j < m_vPredictions[i].size(); j++)
		{
			Prediction p = m_vPredictions[i][j];
			pName = &pObj->m_name[j];

			*pName = p.first;
			pObj->m_prob[j] = p.second;

			int from = pName->find_first_of(' ');
			int to = pName->find_first_of(',');

			*pName = pName->substr(from + 1, to - from); // pObj->m_name[j].length());

			if (j >= NUM_OBJECT_NAME)
				break;
		}

		pObj->m_status = OBJ_COMPLETE;
	}

}

OBJECT* _Universe::addUnknownObject(Mat* pMat, Rect* pRect, vector<Point>* pContour)
{
	if (!pMat)
		return NULL;
	if (!pRect)
		return NULL;
	if (pMat->empty())
		return NULL;
	if (pRect->width <= 0)
		return NULL;
	if (pRect->height <= 0)
		return NULL;

	int i;
	int iVacant;
	OBJECT* pObj;
	uint64_t frameID = get_time_usec();

	iVacant = m_numObj;
	for (i = 0; i < m_numObj; i++)
	{
		pObj = &m_pObjects[i];

		//Record the index of vacancy
		if (pObj->m_status == OBJ_VACANT)
		{
			if (iVacant == m_numObj)
			{
				iVacant = i;
			}

			continue;
		}

		//Compare if already existed
		if (abs(pObj->m_boundBox.x - pRect->x) > m_disparity)
			continue;
		if (abs(pObj->m_boundBox.y - pRect->y) > m_disparity)
			continue;
		if (abs(pObj->m_boundBox.width - pRect->width) > m_disparity)
			continue;
		if (abs(pObj->m_boundBox.height - pRect->height) > m_disparity)
			continue;

		//The region is already under recognizing
		pObj->m_frameID = frameID;
//		pObj->m_status = OBJ_ADDED;

//Update the Rect
		pObj->m_boundBox = *pRect;
		pObj->m_vContours = *pContour;

		return pObj;
	}

	if (iVacant < m_numObj)
	{
		//Change in status comes to the last
		pObj = &m_pObjects[iVacant];
		pObj->m_frameID = frameID;
		pObj->m_boundBox = *pRect;
		pObj->m_Mat = Mat(pRect->width, pRect->height, pMat->type());
		pMat->colRange(pRect->tl().x, pRect->br().x).rowRange(pRect->tl().y,
				pRect->br().y).copyTo(pObj->m_Mat);
		if (pContour)
		{
			pObj->m_vContours = *pContour;
		}
		pObj->m_status = OBJ_ADDED;

		return pObj;
	}

	return NULL;
}

OBJECT* _Universe::addKnownObject(string name, int safetyGrade, Mat* pMat, Rect* pRect, vector<Point>* pContour)
{
	if (!pRect)
		return NULL;
	if (pRect->width <= 0)
		return NULL;
	if (pRect->height <= 0)
		return NULL;

	int i;
	int iVacant;
	OBJECT* pObj;
	uint64_t frameID = get_time_usec();

	iVacant = m_numObj;
	for (i = 0; i < m_numObj; i++)
	{
		pObj = &m_pObjects[i];

		if (pObj->m_status != OBJ_VACANT)
		{
			//Compare if already existed
			if (abs(pObj->m_boundBox.x - pRect->x) > m_disparity)continue;
			if (abs(pObj->m_boundBox.y - pRect->y) > m_disparity)continue;
			if (abs(pObj->m_boundBox.width - pRect->width) > m_disparity)continue;
			if (abs(pObj->m_boundBox.height - pRect->height) > m_disparity)continue;

			//Already existed
			pObj->m_frameID = frameID;
			pObj->m_name[0] = name;
			pObj->m_safetyGrade = safetyGrade;
			pObj->m_boundBox = *pRect;

			return pObj;
		}

		//Record the index of vacancy
		iVacant = i;
		break;
	}

	if (iVacant >= m_numObj)return NULL;

	//Change in status comes to the last
	pObj = &m_pObjects[iVacant];
	pObj->m_frameID = frameID;
	pObj->m_name[0] = name;
	pObj->m_safetyGrade = safetyGrade;
	pObj->m_boundBox = *pRect;
//	pObj->m_Mat = Mat(pRect->width,pRect->height,pMat->type());
//	pMat->colRange(pRect->tl().x,pRect->br().x).rowRange(pRect->tl().y,pRect->br().y).copyTo(pObj->m_Mat);
//	if(pContour)
//	{
//		pObj->m_vContours = *pContour;
//	}
	pObj->m_status = OBJ_COMPLETE;

	return pObj;

}

void _Universe::reset(void)
{
	for (int i = 0; i < m_numObj; i++)
	{
		m_pObjects[i].m_frameID = 0;
		m_pObjects[i].m_status = OBJ_VACANT;
		m_pObjects[i].m_safetyGrade = 0;
		m_pObjects[i].m_vContours.clear();

		for (int j = 0; j < NUM_OBJECT_NAME; j++)
		{
			m_pObjects[i].m_name[j] = "";
			m_pObjects[i].m_prob[j] = 0;
		}
	}

}

bool _Universe::draw(Frame* pFrame, iVec4* pTextPos)
{
	if (pFrame == NULL)
		return false;

	int i;
	OBJECT* pObj;
	Mat* pMat = pFrame->getCMat();
	Scalar color;
	double thickness;

	for (i = 0; i < m_numObj; i++)
	{
		pObj = &m_pObjects[i];

		if (pObj->m_status == OBJ_COMPLETE)
		{
			if (pObj->m_name[0].empty())
				continue;

			if(pObj->m_safetyGrade == 0)
			{
				color = Scalar(0,0,255);
				thickness = 2;
			}
			else if(pObj->m_safetyGrade == 1)
			{
				color = Scalar(0,255,255);
				thickness = 1;
			}
			else
			{
				color = Scalar(0,255,0);
				thickness = 1;
			}

			rectangle(*pMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(), color, thickness);

			putText(*pMat, pObj->m_name[0],
					Point(pObj->m_boundBox.x + pObj->m_boundBox.width / 2,
							pObj->m_boundBox.y + pObj->m_boundBox.height / 2),
					FONT_HERSHEY_SIMPLEX, 0.8, color, thickness);
		}
	}

	putText(*pMat, "Universe FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;


//	vMat = Mat(m_pDD->m_Mat.rows, m_pDD->m_Mat.cols, CV_8UC3, Scalar(0));
//
//	for (i = 0; i < m_pUniverse->m_numObj; i++)
//	{
//		pObj = &m_pUniverse->m_pObjects[i];
//		contours.clear();
//		contours.push_back(pObj->m_vContours);
//
//		//Green
//		if (pObj->m_status == OBJ_COMPLETE)
//		{
//			if (pObj->m_name[0].empty())
//				continue;
//
//			drawContours(vMat, contours, -1, Scalar(0, 255, 0), CV_FILLED);
//
//			rectangle(m_showMat, pObj->m_boundBox.tl(),
//					pObj->m_boundBox.br(), Scalar(0, 255, 0));
//			putText(m_showMat, pObj->m_name[0],
//					Point(pObj->m_boundBox.x + pObj->m_boundBox.width / 2,
//							pObj->m_boundBox.y
//									+ pObj->m_boundBox.height / 2),
//					FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 2);
//		}
//
//		//Yellow
//		if (pObj->m_status == OBJ_CLASSIFYING)
//		{
//			drawContours(vMat, contours, -1, Scalar(0, 255, 255), 1);
//			//			rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(), Scalar(0, 255, 255), 1);
//		}
//
//		//Red
//		if (pObj->m_status == OBJ_ADDED)
//		{
//			drawContours(vMat, contours, -1, Scalar(0, 0, 255), 1);
//			//			rectangle(imMat, pObj->m_boundBox.tl(), pObj->m_boundBox.br(), Scalar(0, 0, 255), 1);
//		}
//	}
//
//	cv::resize(vMat, imMat, Size(m_showMat.cols, m_showMat.rows));
//	cv::addWeighted(m_showMat, 1.0, imMat, 0.25, 0.0, vMat);
//	vMat.copyTo(m_showMat);

}

} /* namespace kai */
