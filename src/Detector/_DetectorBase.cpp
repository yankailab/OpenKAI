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
	m_vClassColor.clear();

	m_overlapMin = 1.0;
	m_pDetIn = NULL;

	m_sizeName = 0.5;
	m_sizeDist = 0.5;
	m_col = Scalar(0,0,0);
	m_bDrawContour = false;
	m_contourBlend = 0.125;
	m_minConfidence = 0.0;

}

_DetectorBase::~_DetectorBase()
{
	m_obj.release();
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

	KISSm(pK, sizeName);
	KISSm(pK, sizeDist);

	F_INFO(pK->v("B", &m_col[0]));
	F_INFO(pK->v("G", &m_col[1]));
	F_INFO(pK->v("R", &m_col[2]));

	KISSm(pK, bDrawContour);
	KISSm(pK, contourBlend);

	int i;
	m_vClassColor.clear();
	string pClassColor[N_CLASS];
	int nClassColor = pK->array("classColor", pClassColor, N_CLASS);
	if(nClassColor > 0)
	{
		for(i=0; i<nClassColor; i++)
		{
			vector<string> vClassColor = splitBy(pClassColor[i], ',');
			Scalar col = m_col;
			if(vClassColor.size() > 0)col[0]=atoi(vClassColor[0].c_str());
			if(vClassColor.size() > 1)col[1]=atoi(vClassColor[1].c_str());
			if(vClassColor.size() > 2)col[2]=atoi(vClassColor[2].c_str());
			m_vClassColor.push_back(col);
		}
	}

	int n = 16;
	F_INFO(pK->v("nObj", &n));
	return m_obj.init(n);

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

bool _DetectorBase::start(void)
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

void _DetectorBase::update(void)
{
	m_obj.update();
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

	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		IF_CONT(pO->m_prob < m_minConfidence);

		Rect r;
		vInt42rect(&pO->m_bbox, &r);

		Scalar oColor = m_col;
		if(m_vClassColor.size() > pO->m_iClass)
			oColor = m_vClassColor[pO->m_iClass];

		if (pO->m_iClass>=0)
		{
			putText(*pMat, pO->m_name,
					Point(r.x + r.width / 2, r.y + r.height / 2),
					FONT_HERSHEY_SIMPLEX, m_sizeName, oColor, 1);
		}

		int bolObs = 1;

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

	return true;
}

}

