/*
 * _GDcam.cpp
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#include "_GDcam.h"

namespace kai
{

_GDcam::_GDcam()
{
	m_pD = NULL;

	m_imgFile = "GDcam_";
	m_alprAPI = "https://api.openalpr.com/v2/recognize?recognize_vehicle=0&country=jp&secret_key=";
	m_gdUpload = "python gdUpload.py";
	m_gdFolderID = "";

}

_GDcam::~_GDcam()
{
}

bool _GDcam::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,imgFile);
	KISSm(pK,alprAPI);
	KISSm(pK,gdUpload);
	KISSm(pK,gdFolderID);

	int jpgQuality = 80;
	pK->v("jpgQuality", &jpgQuality);
	m_vJPGquality.push_back(IMWRITE_JPEG_QUALITY);
	m_vJPGquality.push_back(jpgQuality);

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pD = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pD, iName + " not found");

	return true;
}

bool _GDcam::start(void)
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

void _GDcam::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateShot();

		this->autoFPSto();
	}
}

int _GDcam::check(void)
{
	NULL__(m_pD,-1);
	NULL__(m_pD->m_pVision,-1);
	IF__(m_pD->m_pVision->BGR()->bEmpty(), -1);

	return 0;
}

void _GDcam::updateShot(void)
{
	IF_(check()<0);

	m_fBGR.copy(*m_pD->m_pVision->BGR());
	string fImg = m_imgFile + tFormat() + ".jpg";
	cv::imwrite(fImg, *m_fBGR.m(), m_vJPGquality);

	//ALPR
	FILE *fp;
	char path[1035];
	string alpr = "curl -X POST -F image=@" + fImg + " '" + m_alprAPI + "'";

	fp = popen(alpr.c_str(), "r");
	if(!fp)
		LOG_I("Failed to run command:" + alpr);

	//Upload to Google Drive
	string gdUp = m_gdUpload + " " + fImg + " " + m_gdFolderID;
	system(gdUp.c_str());
}

bool _GDcam::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

bool _GDcam::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}
