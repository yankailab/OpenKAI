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
}

_ArUco::~_ArUco()
{

}

bool _ArUco::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK, dict);

	m_pDict = aruco::getPredefinedDictionary(aruco::DICT_4X4_50);//DICT_APRILTAG_16h5

	return true;
}

bool _ArUco::link(void)
{
	IF_F(!this->_DetectorBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";
	F_ERROR_F(pK->v("_VisionBase",&iName));
	m_pVision = (_VisionBase*)(pK->root()->getChildInstByName(&iName));

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

		this->_DetectorBase::update();
		m_obj.update();
		detect();

		this->autoFPSto();
	}
}

void _ArUco::detect(void)
{
	NULL_(m_pVision);
	Mat m = *m_pVision->BGR()->m();
	IF_(m.empty());

    cv::aruco::detectMarkers(m, m_pDict, m_vvCorner, m_vID);

	OBJECT obj;
	double dx,dy;

	for (int i = 0; i < m_vID.size(); i++)
	{
		obj.init();
		obj.m_tStamp = m_tStamp;
		obj.setTopClass(m_vID[i]);

		Point2f pLT = m_vvCorner[i][0];
		Point2f pRT = m_vvCorner[i][1];
		Point2f pRB = m_vvCorner[i][2];
		Point2f pLB = m_vvCorner[i][3];

		// center position
		obj.m_fBBox.x = (double)(pLT.x + pRT.x + pRB.x + pLB.x)*0.25;
		obj.m_fBBox.y = (double)(pLT.y + pRT.y + pRB.y + pLB.y)*0.25;

		// radius
		dx = obj.m_fBBox.x - pLT.x;
		dy = obj.m_fBBox.y - pLT.y;
		obj.m_fBBox.z = sqrt(dx*dx + dy*dy);

		// angle in deg
		dx = pLB.x - pLT.x;
		dy = pLB.y - pLT.y;
		obj.m_fBBox.w = -atan2(dx,dy) * RAD_DEG;

		obj.m_camSize.x = m.cols;
		obj.m_camSize.y = m.rows;

		add(&obj);
		LOG_I("ID: "+ i2str(obj.m_iClass));
	}
}

bool _ArUco::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	string msg = "nID: " + i2str(m_vID.size());
	pWin->addMsg(&msg);
	IF_T(m_vID.size() <= 0);

	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		Point pCenter = Point(pO->m_fBBox.x, pO->m_fBBox.y);
		circle(*pMat, pCenter, pO->m_fBBox.z, Scalar(0, 255, 0), 2);

		putText(*pMat, i2str(pO->m_iClass) + " / " + i2str(pO->m_fBBox.w),
				pCenter,
				FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 0), 2);

		double rad = -pO->m_fBBox.w * DEG_RAD;
		Point pD = Point(pO->m_fBBox.z*sin(rad), pO->m_fBBox.z*cos(rad));
		line(*pMat, pCenter + pD, pCenter - pD, Scalar(0, 0, 255), 2);
	}

	return true;
}

bool _ArUco::cli(int& iY)
{
	IF_F(!this->_DetectorBase::cli(iY));

	string msg = "| ";
	for (int i = 0; i < m_vID.size(); i++)
	{
		msg += i2str(m_vID[i]) + " | ";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
#endif
