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
	m_classFile = "";
	m_minConfidence = 0.0;
	m_minArea = -1.0;
	m_maxArea = -1.0;
	m_maxW = -1.0;
	m_maxH = -1.0;
	m_nClass = 0;
	m_obj.reset();
	m_roi.init();
	m_roi.z = 1.0;
	m_roi.w = 1.0;

	m_drawVscale = 1.0;
	m_bDrawSegment = false;
	m_segmentBlend = 0.125;
	m_bDrawStatistics = false;
	m_classLegendPos.x = 25;
	m_classLegendPos.y = 100;
	m_classLegendPos.z = 15;
	m_bDrawObjClass = false;
}

_DetectorBase::~_DetectorBase()
{
}

bool _DetectorBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	//general
	KISSm(pK, minConfidence);
	KISSm(pK, minArea);
	KISSm(pK, maxArea);
	KISSm(pK, maxW);
	KISSm(pK, maxH);
	m_obj.reset();

	//model
	string modelDir = "";
	F_INFO(pK->v("modelDir", &modelDir));

	KISSm(pK, modelFile);
	KISSm(pK, trainedFile);
	KISSm(pK, meanFile);
	KISSm(pK, labelFile);
	KISSm(pK, classFile);

	m_modelFile = modelDir + m_modelFile;
	m_trainedFile = modelDir + m_trainedFile;
	m_meanFile = modelDir + m_meanFile;
	m_labelFile = modelDir + m_labelFile;

	//class
	if(!m_classFile.empty())
	{
		m_classFile = modelDir + m_classFile;
		ifstream ifs(m_classFile.c_str());
	    IF_Fl(!ifs.is_open(),"File " + m_classFile + " not found");

	    string line;
		while (std::getline(ifs, line))
		{
			OBJECT_CLASS oc;
			oc.init();
			oc.m_name = line;
			m_vClass.push_back(oc);
		}

		m_nClass = m_vClass.size();
		ifs.close();
	}
	else
	{
		string pClassList[OBJECT_N_CLASS];
		m_nClass = pK->array("classList", pClassList, OBJECT_N_CLASS);

		for(int i=0; i<m_nClass; i++)
		{
			OBJECT_CLASS oc;
			oc.init();
			oc.m_name = pClassList[i];
			m_vClass.push_back(oc);
		}
	}

	//draw
	KISSm(pK, bDrawSegment);
	KISSm(pK, segmentBlend);
	KISSm(pK, bDrawStatistics);
	KISSm(pK, drawVscale);
	KISSm(pK, bDrawObjClass);

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

void _DetectorBase::updateStatistics(void)
{
	int i;
	for(i=0; i<m_nClass; i++)
	{
		m_vClass[i].m_n = 0;
	}

	for(i=0; i<size(); i++)
	{
		OBJECT* pO = at(i);

		IF_CONT(pO->m_topClass >= m_nClass);
		IF_CONT(pO->m_topClass < 0);

		m_vClass[pO->m_topClass].m_n++;
	}
}

int _DetectorBase::getClassIdx(string& className)
{
	for(int i=0; i<m_nClass; i++)
	{
		if(m_vClass[i].m_name == className)return i;
	}

	return -1;
}

string _DetectorBase::getClassName(int iClass)
{
	if(iClass < 0)return "";
	if(iClass >= m_nClass)return "";

	return m_vClass[iClass].m_name;
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

	double area = pNewO->area();
	IF_N(m_minArea >= 0 && area < m_minArea);
	IF_N(m_maxArea >= 0 && area > m_maxArea);
	IF_N(m_maxW >= 0 && pNewO->width() > m_maxW);
	IF_N(m_maxH >= 0 && pNewO->height() > m_maxH);

	return m_obj.add(pNewO);
}

void _DetectorBase::merge(_DetectorBase* pO)
{
	NULL_(pO);

	OBJECT* pObj;
	int i=0;
	while((pObj = pO->at(i++)) != NULL)
	{
		add(pObj);
	}
}

bool _DetectorBase::draw(void)
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

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
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

		//bb
		Rect r = pO->getRect(cs);
		rectangle(*pMat, r, oCol, 1);

		//class
		if(m_bDrawObjClass)
		{
			string oName = m_vClass[iClass].m_name;
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
		OBJECT_CLASS* pC = &m_vClass[i];
		col = colStep * i;
		oCol = Scalar((col+85)%255, (col+170)%255, col) + bCol;

		putText(*pMat, pC->m_name + ": " + i2str(pC->m_n),
				Point(m_classLegendPos.x, m_classLegendPos.y + i*m_classLegendPos.z),
				FONT_HERSHEY_SIMPLEX, 0.5, oCol, 1);
	}

	return true;
}

bool _DetectorBase::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;
	C_MSG("nObj=" + i2str(m_obj.size()));

	return true;
}

}
