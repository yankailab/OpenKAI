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
	m_pDB = NULL;

	m_modelFile = "";
	m_trainedFile = "";
	m_meanFile = "";
	m_classFile = "";
	m_minConfidence = 0.0;
	m_minArea = -1.0;
	m_maxArea = -1.0;
	m_minW = -1.0;
	m_maxW = -1.0;
	m_minH = -1.0;
	m_maxH = -1.0;
	m_nClass = 0;
	m_bMerge = false;
	m_mergeOverlap = 0.8;
	m_bbScale = -1.0;

	m_bDrawStatistics = false;
	m_classLegendPos.x = 25;
	m_classLegendPos.y = 100;
	m_classLegendPos.z = 15;
	m_bDrawClass = false;
	m_bDrawText = false;
	m_bDrawPos = false;

	resetObj();
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
	KISSm(pK, minW);
	KISSm(pK, minH);
	KISSm(pK, maxW);
	KISSm(pK, maxH);
	KISSm(pK, bMerge);
	KISSm(pK, mergeOverlap);
	KISSm(pK, bbScale);

	resetObj();

	//model
	string modelDir = "";
	F_INFO(pK->v("modelDir", &modelDir));

	KISSm(pK, modelFile);
	KISSm(pK, trainedFile);
	KISSm(pK, meanFile);
	KISSm(pK, classFile);

	m_modelFile = modelDir + m_modelFile;
	m_trainedFile = modelDir + m_trainedFile;
	m_meanFile = modelDir + m_meanFile;

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
	KISSm(pK, bDrawStatistics);
	KISSm(pK, bDrawClass);
	KISSm(pK, bDrawText);
	KISSm(pK, bDrawPos);

	string iName = "";
	F_INFO(pK->v("_VisionBase", &iName));
	m_pVision = (_VisionBase*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("_DetectorBase", &iName);
	m_pDB = (_DetectorBase*) (pK->root()->getChildInst(iName));

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

void _DetectorBase::resetObj(void)
{
	m_iSwitch = 0;
	updateObj();
	m_pPrev->reset();
	m_pNext->reset();
}

void _DetectorBase::updateObj(void)
{
	m_iSwitch = 1 - m_iSwitch;
	m_pPrev = &m_pObj[m_iSwitch];
	m_pNext = &m_pObj[1 - m_iSwitch];
	m_pNext->reset();
}

int _DetectorBase::size(void)
{
	return m_pPrev->size();
}

OBJECT* _DetectorBase::at(int i)
{
	return m_pPrev->at(i);
}

OBJECT* _DetectorBase::add(OBJECT* pNewO)
{
	NULL_N(pNewO);

	float area = pNewO->area();
	IF_N(m_minArea >= 0 && area < m_minArea);
	IF_N(m_maxArea >= 0 && area > m_maxArea);
	IF_N(m_minW >= 0 && pNewO->width() < m_minW);
	IF_N(m_maxW >= 0 && pNewO->width() > m_maxW);
	IF_N(m_minH >= 0 && pNewO->height() < m_minW);
	IF_N(m_maxH >= 0 && pNewO->height() > m_maxH);

	if(m_bbScale > 0.0)
	{
		pNewO->m_bb = bbScale(pNewO->m_bb, m_bbScale);
		pNewO->m_bb.constrain(0.0, 1.0);
	}

	if(m_bMerge)
	{
		vFloat4 BB = pNewO->m_bb;

		for(int i=0; i<m_pNext->m_nObj; i++)
		{
			OBJECT* pO = &m_pNext->m_pObj[i];
			IF_CONT(pO->m_topClass != pNewO->m_topClass);
			IF_CONT(nIoU(BB, pO->m_bb) < m_mergeOverlap);

			pO->m_bb.x = small(pNewO->m_bb.x, pO->m_bb.x);
			pO->m_bb.y = small(pNewO->m_bb.y, pO->m_bb.y);
			pO->m_bb.z = big(pNewO->m_bb.z, pO->m_bb.z);
			pO->m_bb.w = big(pNewO->m_bb.w, pO->m_bb.w);
			pO->m_topProb = big(pNewO->m_topProb, pO->m_topProb);
			pO->m_mClass |= pNewO->m_mClass;

			return pO;
		}
	}

	return m_pNext->add(pNewO);
}

void _DetectorBase::pipeIn(void)
{
	NULL_(m_pDB);

	OBJECT* pO;
	int i=0;
	while((pO = m_pDB->at(i++)))
	{
		add(pO);
	}
}

bool _DetectorBase::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Scalar oCol;
	Scalar bCol = Scalar(100,100,100);
	int col;
	int colStep = 255/m_nClass;
	OBJECT* pO;
	int i=0;
	while((pO = at(i++)) != NULL)
	{
		int iClass = pO->m_topClass;

		col = colStep * iClass;
		oCol = Scalar((col+85)%255, (col+170)%255, col) + bCol;

		//bb
		Rect r = convertBB<vInt4>(convertBB(pO->m_bb, cs));
		rectangle(*pMat, r, oCol, 1);

		//position
		if(m_bDrawPos)
		{
			putText(*pMat, f2str(pO->m_dist),
					Point(r.x + 15, r.y + 25),
					FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
		}

		//class
		if(m_bDrawClass && iClass < m_nClass && iClass >= 0)
		{
			string oName = m_vClass[iClass].m_name;
			if (oName.length()>0)
			{
				putText(*pMat, oName,
						Point(r.x + 15, r.y + 50),
						FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
			}
		}

		//text
		if(m_bDrawText)
		{
			string oName = string(pO->m_pText);
			if (oName.length()>0)
			{
				putText(*pMat, oName,
						Point(r.x + 15, r.y + 50),
						FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
			}
		}
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
	C_MSG("nObj=" + i2str(size()));

	return true;
}

}
