/*
 * _ArUco.cpp
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#include "_ArUco.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

_ArUco::_ArUco()
{
	m_pVision = NULL;
	m_dict = aruco::DICT_4X4_50;//DICT_APRILTAG_16h5;
	m_minArea = DBL_MIN;
	m_maxArea = DBL_MAX;

}

_ArUco::~_ArUco()
{

}

bool _ArUco::init(void* pKiss)
{
	IF_F(!this->_ObjectBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK, dict);

	m_pDict = aruco::getPredefinedDictionary(m_dict);

	//link
	string iName = "";
	F_ERROR_F(pK->v("_VisionBase",&iName));
	m_pVision = (_VisionBase*)(pK->root()->getChildInst(iName));

	return true;
}

bool _ArUco::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _ArUco::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_ObjectBase::update();
		detect();
		m_obj.update();

		if(m_bGoSleep)
		{
			m_obj.m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

void _ArUco::detect(void)
{
	NULL_(m_pVision);
	Mat m = *m_pVision->BGR()->m();
	IF_(m.empty());

    std::vector<int> vID;
    std::vector<std::vector<cv::Point2f> > vvCorner;
    cv::aruco::detectMarkers(m, m_pDict, vvCorner, vID);

	OBJECT obj;
	double dx,dy;

	for (int i = 0; i < vID.size(); i++)
	{
		obj.init();
		obj.m_tStamp = m_tStamp;
		obj.setTopClass(vID[i],1.0);

		Point2f pLT = vvCorner[i][0];
		Point2f pRT = vvCorner[i][1];
		Point2f pRB = vvCorner[i][2];
		Point2f pLB = vvCorner[i][3];

		// center position
		obj.m_bb.x = (double)(pLT.x + pRT.x + pRB.x + pLB.x)*0.25;
		obj.m_bb.y = (double)(pLT.y + pRT.y + pRB.y + pLB.y)*0.25;

		// radius
		dx = obj.m_bb.x - pLT.x;
		dy = obj.m_bb.y - pLT.y;
		obj.m_bb.z = sqrt(dx*dx + dy*dy);

		// angle in deg
		dx = pLB.x - pLT.x;
		dy = pLB.y - pLT.y;
		obj.m_bb.w = -atan2(dx,dy) * RAD_DEG;

		add(&obj);
		LOG_I("ID: "+ i2str(obj.m_topClass));
	}
}

bool _ArUco::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	string msg = "nTag: " + i2str(this->size());
	pWin->addMsg(&msg);
	IF_T(this->size() <= 0);

	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		Point pCenter = Point(pO->m_bb.x, pO->m_bb.y);
		circle(*pMat, pCenter, pO->m_bb.z, Scalar(0, 255, 0), 2);

		putText(*pMat, i2str(pO->m_topClass) + " / " + i2str(pO->m_bb.w),
				pCenter,
				FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 0), 2);

		double rad = -pO->m_bb.w * DEG_RAD;
		Point pD = Point(pO->m_bb.z*sin(rad), pO->m_bb.z*cos(rad));
		line(*pMat, pCenter + pD, pCenter - pD, Scalar(0, 0, 255), 2);
	}

	return true;
}

bool _ArUco::cli(int& iY)
{
	IF_F(!this->_ObjectBase::cli(iY));

	string msg = "| ";
	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		msg += i2str(pO->m_topClass) + " | ";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
#endif
