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
	m_dMaxTrack = 1.0;
	m_minConfidence = 0.0;
	m_minArea = 0.0;
	m_maxArea = 1.0;
	m_nClass = OBJECT_N_CLASS;
	m_obj.reset();
	m_roi.init();
	m_roi.z = 1.0;
	m_roi.w = 1.0;
	m_trackID = 1;

	m_drawVscale = 1.0;
	m_bDrawSegment = false;
	m_segmentBlend = 0.125;
	m_bDrawStatistics = false;
	m_classLegendPos.x = 25;
	m_classLegendPos.y = 100;
	m_classLegendPos.z = 15;
	m_bDrawObjClass = false;
	m_bDrawObjVtrack = false;
}

_ObjectBase::~_ObjectBase()
{
}

bool _ObjectBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	//general
	KISSm(pK, dMaxTrack);
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
	KISSm(pK, drawVscale);
	KISSm(pK, bDrawObjClass);
	KISSm(pK, bDrawObjVtrack);

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
	F_INFO(pK->v("rX", &m_roi.x));
	F_INFO(pK->v("rY", &m_roi.y));
	F_INFO(pK->v("rZ", &m_roi.z));
	F_INFO(pK->v("rW", &m_roi.w));

	//link
	string iName = "";
	F_INFO(pK->v("_VisionBase", &iName));
	m_pVision = (_VisionBase*) (pK->root()->getChildInst(iName));

	return true;
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
	for(int i=0; i<m_nClass; i++)
	{
		if(m_pClassStatis[i].m_name == className)return i;
	}

	return -1;
}

string _ObjectBase::getClassName(int iClass)
{
	if(iClass < 0)return "";
	if(iClass >= m_nClass)return "";

	return m_pClassStatis[iClass].m_name;
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

	double area = pNewO->m_bb.area();
	IF_N(area < m_minArea);
	IF_N(area > m_maxArea);

	OBJECT* pO;
	double minD = DBL_MAX;
	int iD = -1;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		double dX = pO->m_bb.midX() - pNewO->m_bb.midX();
		double dY = pO->m_bb.midY() - pNewO->m_bb.midY();
		double d = sqrt(dX * dX + dY * dY);

		IF_CONT(d > m_dMaxTrack);
		IF_CONT(d > minD);
		IF_CONT(pO->m_topClass != pNewO->m_topClass);

		minD = d;
		iD = i - 1;
	}

	pO = m_obj.at(iD);
	if(pO)
	{
		vInt2 cSize = m_pVision->getSize();

		pNewO->m_vTrack.x = (pNewO->m_bb.midX() - pO->m_bb.midX()) / (double)cSize.x;
		pNewO->m_vTrack.y = (pNewO->m_bb.midY() - pO->m_bb.midY()) / (double)cSize.y;

		if(pO->m_trackID>0)
			pNewO->m_trackID = pO->m_trackID;
		else
			pNewO->m_trackID = m_trackID++;
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

bool _ObjectBase::draw(void)
{
	IF_F(!this->_ThreadBase::draw());

	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	Mat bg;
	if (m_bDrawSegment)
	{
		bg = Mat::zeros(Size(pMat->cols, pMat->rows), CV_8UC3);
	}

	vInt2 cSize;
	cSize.x = pMat->cols;
	cSize.y = pMat->rows;
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
		oCol = Scalar((col+85)%255, (col+170)%255, col) + bCol;

		vInt4 iBB = pO->iBBox(cSize);
		Point pC = Point(iBB.midX(), iBB.midY());

		//trackID
		if(pO->m_trackID > 0)
			oCol += Scalar(100, 100, 100);

		//bbox
		Rect r;
		vInt42rect(iBB, r);
		rectangle(*pMat, r, oCol, 1);

		//vTrack
		if(m_bDrawObjVtrack)
		{
			Point pV = Point(pO->m_vTrack.x * cSize.x * m_drawVscale,
							 pO->m_vTrack.y * cSize.x * m_drawVscale);
			line(*pMat, pC, pC - pV, oCol, 1);
		}

		//class
		if(m_bDrawObjClass)
		{
			string oName = m_pClassStatis[iClass].m_name;
			if (oName.length()>0)
			{
				putText(*pMat, oName,
						Point(r.x + 15, r.y + 25),
						FONT_HERSHEY_SIMPLEX, 0.8, oCol, 1);
			}
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
		oCol = Scalar((col+85)%255, (col+170)%255, col) + bCol;

		putText(*pMat, pC->m_name + ": " + i2str(pC->m_n),
				Point(m_classLegendPos.x, m_classLegendPos.y + i*m_classLegendPos.z),
				FONT_HERSHEY_SIMPLEX, 0.5, oCol, 1);
	}

	return true;
}

bool _ObjectBase::cli(int& iY)
{
	IF_F(!this->_ThreadBase::cli(iY));

	string msg = "nObj=" + i2str(m_obj.size());
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
