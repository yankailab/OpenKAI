/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_DetectorBase.h"

namespace kai
{

_DetectorBase::_DetectorBase()
{
	m_pVision = NULL;
	m_modelFile = "";
	m_trainedFile = "";
	m_meanFile = "";
	m_labelFile = "";
	m_overlapMin = 1.0;
//	m_pDetIn = NULL;
	m_minConfidence = 0.0;
	m_minArea = 0.0;
	m_maxArea = 1.0;
	m_nClass = DETECTOR_N_CLASS;
	m_tStamp = 0;
	m_obj.reset();

	m_bActive = true;
	m_bReady = false;
	m_mode = thread;

	m_drawVeloScale = 1.0;
	m_bDrawSegment = false;
	m_segmentBlend = 0.125;
	m_bDrawStatistics = false;
	m_classLegendPos.x = 25;
	m_classLegendPos.y = 150;
	m_classLegendPos.z = 15;
}

_DetectorBase::~_DetectorBase()
{
}

bool _DetectorBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	//mode
	string iName = "thread";
	F_INFO(pK->v("mode", &iName));
	if (iName == "noThread")
	{
		m_mode = noThread;
		bSetActive(false);
	}
	else if (iName == "batch")
	{
		m_mode = batch;
		bSetActive(false);
	}

	//general
	KISSm(pK, overlapMin);
	KISSm(pK, minConfidence);
	KISSm(pK, minArea);
	KISSm(pK, maxArea);
	m_obj.reset();

	//model
	string modelDir = "";
	F_INFO(pK->v("modelDir", &modelDir));

	KISSm(pK, modelFile);
	KISSm(pK, trainedFile);
	KISSm(pK, meanFile);
	KISSm(pK, labelFile);

	m_modelFile = modelDir + m_modelFile;
	m_trainedFile = modelDir + m_trainedFile;
	m_meanFile = modelDir + m_meanFile;
	m_labelFile = modelDir + m_labelFile;

	//draw
	KISSm(pK, bDrawSegment);
	KISSm(pK, segmentBlend);
	KISSm(pK, bDrawStatistics);
	KISSm(pK, drawVeloScale);

	string pClassList[DETECTOR_N_CLASS];
	m_nClass = pK->array("classList", pClassList, DETECTOR_N_CLASS);

	//statistics
	int i;
	for(i=0; i<m_nClass; i++)
	{
		m_pClassStatis[i].init();
		m_pClassStatis[i].m_name = pClassList[i];
	}

	return true;
}

bool _DetectorBase::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("_VisionBase", &iName));
	m_pVision = (_VisionBase*) (pK->root()->getChildInstByName(&iName));

//	iName = "";
//	F_INFO(pK->v("_DetectorIn", &iName));
//	m_pDetIn = (_DetectorBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void _DetectorBase::update(void)
{
	m_tStamp = getTimeUsec();
}

void _DetectorBase::updateStatistics(void)
{
	int i;
	for(i=0; i<m_nClass; i++)
	{
		m_pClassStatis[i].m_n = 0;
	}

	for(i=0; i<size(); i++)
	{
		OBJECT* pO = at(i);

		IF_CONT(pO->m_iClass >= m_nClass);
		IF_CONT(pO->m_iClass < 0);

		m_pClassStatis[pO->m_iClass].m_n++;
	}
}

int _DetectorBase::getClassIdx(string& className)
{
	return -1;
}

string _DetectorBase::getClassName(int iClass)
{
	return "";
}

bool _DetectorBase::bReady(void)
{
	return m_bReady;
}

int _DetectorBase::size(void)
{
	return m_obj.size();
}

OBJECT* _DetectorBase::at(int i)
{
	return m_obj.at(i);
}

OBJECT* _DetectorBase::add(OBJECT* pNewO)
{
	NULL_N(pNewO);

	double area = pNewO->m_fBBox.area();
	IF_N(area < m_minArea);
	IF_N(area > m_maxArea);

	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		IF_CONT(overlapRatio(&pO->m_bbox, &pNewO->m_bbox) < m_overlapMin);
		IF_CONT(pO->m_iClass != pNewO->m_iClass);
//		IF_CONT((pO->m_mClass | pNewO->m_mClass) == 0);

		pNewO->m_bTracked = true;
		pNewO->m_velo.x = pNewO->m_bbox.midX() - pO->m_bbox.midX();
		pNewO->m_velo.y = pNewO->m_bbox.midY() - pO->m_bbox.midY();
		break;
	}

	return m_obj.add(pNewO);
}

//void _DetectorBase::mergeDetector(void)
//{
//	NULL_(m_pDetIn);
//
//	OBJECT_DARRAY* pOA = &m_pDetIn->m_obj;
//	OBJECT* pO;
//	int i=0;
//	while((pO = pOA->at(i++)) != NULL)
//	{
//		addOrUpdate(pO);
//	}
//}

void _DetectorBase::bSetActive(bool bActive)
{
	m_bActive = bActive;
}

bool _DetectorBase::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	IF_T(!m_bActive);

	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	Mat bg;
	if (m_bDrawSegment)
	{
		bg = Mat::zeros(Size(pMat->cols, pMat->rows), CV_8UC3);
	}

	Scalar oCol;
	Scalar bCol = Scalar(100,100,100);
	int col;
	int colStep = 255/m_nClass;
	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		int iClass = pO->m_iClass;
		IF_CONT(iClass >= m_nClass);
		IF_CONT(iClass < 0);

		col = colStep * iClass;
		oCol = Scalar(col, (col+85)%255, (col+170)%255) + bCol;

		//draw bbox
		Rect r;
		vInt42rect(&pO->m_bbox, &r);
		rectangle(*pMat, r, oCol, 2);

		//draw velocity
		Point pC = Point(pO->m_bbox.midX(), pO->m_bbox.midY());
		Point pV = Point(pO->m_velo.x * m_drawVeloScale, pO->m_velo.y * m_drawVeloScale);
		circle(*pMat, pC, 3, oCol, 2);
		line(*pMat, pC, pC - pV, oCol, 2);

		//draw name
		string oName = m_pClassStatis[iClass].m_name;
		if (oName.length()>0)
		{
			putText(*pMat, oName,
					Point(r.x + 15, r.y + 25),
					FONT_HERSHEY_SIMPLEX, 0.8, oCol, 2);
		}
	}

	if (m_bDrawSegment)
	{
		cv::addWeighted(*pMat, 1.0, bg, m_segmentBlend, 0.0, *pMat);
	}

	IF_T(!m_bDrawStatistics);
	updateStatistics();

	for(i=0; i<m_nClass; i++)
	{
		CLASS_STATISTICS* pC = &m_pClassStatis[i];
		col = colStep * i;
		oCol = Scalar(col, (col+85)%255, (col+170)%255) + bCol;

		putText(*pMat, pC->m_name + ": " + i2str(pC->m_n),
				Point(m_classLegendPos.x, m_classLegendPos.y + i*m_classLegendPos.z),
				FONT_HERSHEY_SIMPLEX, 0.5, oCol, 1);
	}

	return true;
}

}

