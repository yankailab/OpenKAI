/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_ObjectBase.h"

namespace kai
{

_ObjectBase::_ObjectBase()
{
	m_pVision = NULL;
	m_modelFile = "";
	m_trainedFile = "";
	m_meanFile = "";
	m_labelFile = "";
	m_minOverlap = 1.0;
	m_minConfidence = 0.0;
	m_minArea = 0.0;
	m_maxArea = 1.0;
	m_nClass = OBJECT_N_CLASS;
	m_tStamp = 0;
	m_obj.reset();

	m_bActive = true;
	m_bReady = false;
	m_mode = det_thread;

	m_drawVeloScale = 1.0;
	m_bDrawSegment = false;
	m_segmentBlend = 0.125;
	m_bDrawStatistics = false;
	m_classLegendPos.x = 25;
	m_classLegendPos.y = 150;
	m_classLegendPos.z = 15;
}

_ObjectBase::~_ObjectBase()
{
}

bool _ObjectBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	//mode
	string iName = "thread";
	F_INFO(pK->v("mode", &iName));
	if (iName == "noThread")
	{
		m_mode = det_noThread;
		bSetActive(false);
	}
	else if (iName == "batch")
	{
		m_mode = det_batch;
		bSetActive(false);
	}

	//general
	KISSm(pK, minOverlap);
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

	string pClassList[OBJECT_N_CLASS];
	m_nClass = pK->array("classList", pClassList, OBJECT_N_CLASS);

	//statistics
	int i;
	for(i=0; i<m_nClass; i++)
	{
		m_pClassStatis[i].init();
		m_pClassStatis[i].m_name = pClassList[i];
	}

	//ROI
	Kiss* pR = pK->o("ROI");
	NULL_T(pR);
	Kiss** pItrR = pR->getChildItr();
	DETECT_ROI R;
	i = 0;
	while (pItrR[i])
	{
		pR = pItrR[i++];
		R.init();
		pR->v("x", &R.m_roi.x);
		pR->v("y", &R.m_roi.y);
		pR->v("z", &R.m_roi.z);
		pR->v("w", &R.m_roi.w);
		m_vROI.push_back(R);
	}

	if(m_vROI.size()<=0)
	{
		R.init();
		R.m_roi.z = 1.0;
		R.m_roi.w = 1.0;
		m_vROI.push_back(R);
	}

	return true;
}

bool _ObjectBase::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("_VisionBase", &iName));
	m_pVision = (_VisionBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void _ObjectBase::update(void)
{
	m_tStamp = getTimeUsec();
}

void _ObjectBase::updateStatistics(void)
{
	int i;
	for(i=0; i<m_nClass; i++)
	{
		m_pClassStatis[i].m_n = 0;
	}

	for(i=0; i<size(); i++)
	{
		OBJECT* pO = at(i);

		IF_CONT(pO->m_topClass >= m_nClass);
		IF_CONT(pO->m_topClass < 0);

		m_pClassStatis[pO->m_topClass].m_n++;
	}
}

int _ObjectBase::getClassIdx(string& className)
{
	return -1;
}

string _ObjectBase::getClassName(int iClass)
{
	return "";
}

bool _ObjectBase::bReady(void)
{
	return m_bReady;
}

int _ObjectBase::size(void)
{
	return m_obj.size();
}

OBJECT* _ObjectBase::at(int i)
{
	return m_obj.at(i);
}

OBJECT* _ObjectBase::add(OBJECT* pNewO)
{
	NULL_N(pNewO);

	double area = pNewO->m_fBBox.area();
	IF_N(area < m_minArea);
	IF_N(area > m_maxArea);

	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		IF_CONT(overlapRatio(&pO->m_bbox, &pNewO->m_bbox) < m_minOverlap);
		IF_CONT(pO->m_topClass != pNewO->m_topClass);

		pNewO->m_velo.x = pNewO->m_bbox.midX() - pO->m_bbox.midX();
		pNewO->m_velo.y = pNewO->m_bbox.midY() - pO->m_bbox.midY();
		break;
	}

	return m_obj.add(pNewO);
}

void _ObjectBase::merge(_ObjectBase* pO)
{
	NULL_(pO);

	OBJECT* pObj;
	int i=0;
	while((pObj = pO->at(i++)) != NULL)
	{
		add(pObj);
	}
}

void _ObjectBase::bSetActive(bool bActive)
{
	m_bActive = bActive;
}

bool _ObjectBase::draw(void)
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
		int iClass = pO->m_topClass;
		IF_CONT(iClass >= m_nClass);
		IF_CONT(iClass < 0);

		col = colStep * iClass;
		oCol = Scalar(col, (col+85)%255, (col+170)%255) + bCol;

		//draw bbox
		Rect r;
		vInt42rect(pO->m_bbox, r);
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

bool _ObjectBase::cli(int& iY)
{
	IF_F(!this->_ThreadBase::cli(iY));

	string msg = "nObj="+m_obj.size();
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
