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
	for (int i = 0; i < m_vID.size(); i++)
	{
		obj.init();
		obj.m_tStamp = m_tStamp;
		obj.setTopClass(m_vID[i]);

//		obj.m_fBBox.x = (double)vvCorner;
//		obj.m_fBBox.y = (double)pYO->m_t;
//		obj.m_fBBox.z = (double)pYO->m_r;
//		obj.m_fBBox.w = (double)pYO->m_b;
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

//	aruco::drawDetectedMarkers(*pMat, m_vvCorner, m_vID);

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
