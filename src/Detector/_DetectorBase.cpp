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
	m_vClassDraw.clear();

	m_overlapMin = 1.0;
	m_pDetIn = NULL;

	m_defaultDrawTextSize = 0.5;
	m_defaultDrawColor = Scalar(0,0,0);
	m_bDrawContour = false;
	m_contourBlend = 0.125;
	m_minConfidence = 0.0;
	m_obj.reset();

	m_classDrawPos.x = 50;
	m_classDrawPos.y = 50;
	m_classDrawPos.z = 50;

}

_DetectorBase::~_DetectorBase()
{
}

bool _DetectorBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	//Setup model
	string modelDir = "";
	string presetDir = "";

	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	F_INFO(pK->v("modelDir", &modelDir));

	KISSm(pK, modelFile);
	KISSm(pK, trainedFile);
	KISSm(pK, meanFile);
	KISSm(pK, labelFile);

	m_modelFile = modelDir + m_modelFile;
	m_trainedFile = modelDir + m_trainedFile;
	m_meanFile = modelDir + m_meanFile;
	m_labelFile = modelDir + m_labelFile;

	KISSm(pK, overlapMin);
	KISSm(pK, minConfidence);
	KISSm(pK, defaultDrawTextSize);

	F_INFO(pK->v("B", &m_defaultDrawColor[0]));
	F_INFO(pK->v("G", &m_defaultDrawColor[1]));
	F_INFO(pK->v("R", &m_defaultDrawColor[2]));

	KISSm(pK, bDrawContour);
	KISSm(pK, contourBlend);

	int i;
	m_vClassDraw.clear();
	CLASS_DRAW cd;
	string pClassColor[N_CLASS];
	int nClassColor = pK->array("classColor", pClassColor, N_CLASS);
	if(nClassColor > 0)
	{
		for(i=0; i<nClassColor; i++)
		{
			vector<string> vClassColor = splitBy(pClassColor[i], ',');
			cd.init();
			cd.m_colorBBox = m_defaultDrawColor;
			if(vClassColor.size() > 0)cd.m_colorBBox[0]=atoi(vClassColor[0].c_str());
			if(vClassColor.size() > 1)cd.m_colorBBox[1]=atoi(vClassColor[1].c_str());
			if(vClassColor.size() > 2)cd.m_colorBBox[2]=atoi(vClassColor[2].c_str());
			if(vClassColor.size() > 3)
			{
				cd.m_bDraw = (atoi(vClassColor[3].c_str())==0)?false:true;
			}

			m_vClassDraw.push_back(cd);
		}
	}

	int pInfoPos[3];
	int nInfoPos = pK->array("drawInfoPos", pInfoPos, 3);
	if(nInfoPos > 0)m_classDrawPos.x = pInfoPos[0];
	if(nInfoPos > 1)m_classDrawPos.y = pInfoPos[1];
	if(nInfoPos > 2)m_classDrawPos.z = pInfoPos[2];

	m_obj.reset();

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

	iName = "";
	F_INFO(pK->v("_DetectorIn", &iName));
	m_pDetIn = (_DetectorBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

int _DetectorBase::size(void)
{
	return m_obj.size();
}

OBJECT* _DetectorBase::at(int i)
{
	return m_obj.at(i);
}

OBJECT* _DetectorBase::add(OBJECT* pNewObj)
{
	return m_obj.add(pNewObj);
}

void _DetectorBase::addOrUpdate(OBJECT* pNewObj)
{
	NULL_(pNewObj);

	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		IF_CONT(overlapRatio(&pO->m_bbox, &pNewObj->m_bbox) < m_overlapMin);

		*pO = *pNewObj;
		return;
	}

	m_obj.add(pNewObj);
}

void _DetectorBase::mergeDetector(void)
{
	NULL_(m_pDetIn);

	OBJECT_DARRAY* pOA = &m_pDetIn->m_obj;
	OBJECT* pO;
	int i=0;
	while((pO = pOA->at(i++)) != NULL)
	{
		addOrUpdate(pO);
	}
}

bool _DetectorBase::draw(void)
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

	int i;
	for(i=0;i<m_vClassDraw.size();i++)
		m_vClassDraw[i].m_n = 0;

	OBJECT* pO;
	i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		IF_CONT(pO->m_prob < m_minConfidence);

		Scalar oColor = m_defaultDrawColor;
		CLASS_DRAW cd;
		if(pO->m_iClass < m_vClassDraw.size())
		{
			cd = m_vClassDraw[pO->m_iClass];
			IF_CONT(!cd.m_bDraw);
			oColor = cd.m_colorBBox;
			m_vClassDraw[pO->m_iClass].m_n++;
			if(pO->m_name.length()>0)
			{
				m_vClassDraw[pO->m_iClass].m_name = pO->m_name;
			}
		}

		Rect r;
		vInt42rect(&pO->m_bbox, &r);
		if (pO->m_name.length()>0)
		{
			putText(*pMat, pO->m_name,
					Point(r.x + 25, r.y + 25),
					FONT_HERSHEY_SIMPLEX, m_defaultDrawTextSize, oColor, 2);
		}

		int bolObs = 2;
		if (m_bDrawContour)
		{
			drawContours(bg, vector<vector<Point> >(1, pO->m_contour), -1,
					oColor, CV_FILLED, 8);
		}
		else
		{
			rectangle(*pMat, r, oColor, bolObs);
		}
	}

	if (m_bDrawContour)
	{
		cv::addWeighted(*pMat, 1.0, bg, m_contourBlend, 0.0, *pMat);
	}

	int j=0;
	for(i=0;i<m_vClassDraw.size();i++)
	{
		CLASS_DRAW cd = m_vClassDraw[i];
		IF_CONT(!cd.m_bDraw);

		putText(*pMat, cd.m_name + ": " + i2str(cd.m_n),
				Point(m_classDrawPos.x,m_classDrawPos.y+j*m_classDrawPos.z),
				FONT_HERSHEY_SIMPLEX, m_defaultDrawTextSize, cd.m_colorBBox, 2);
		j++;
	}

	return true;
}

}

