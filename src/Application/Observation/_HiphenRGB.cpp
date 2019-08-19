/*
 * _HiphenRGB.cpp
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#include "_HiphenRGB.h"

namespace kai
{

_HiphenRGB::_HiphenRGB()
{
	m_iImg = 0;
	m_LL.init();

	m_pGPS = NULL;
	m_pCam = NULL;
	m_pHiphen = NULL;

	m_quality = 100;
	m_bFlip = false;

}

_HiphenRGB::~_HiphenRGB()
{
}

bool _HiphenRGB::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("bFlip", &m_bFlip);
	pK->v("quality", &m_quality);
	m_compress.push_back(IMWRITE_JPEG_QUALITY);
	m_compress.push_back(m_quality);

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_GPS", &iName));
	m_pGPS = (_GPS*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pGPS, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_Camera", &iName));
	m_pCam = (_Camera*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pCam, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_HiphenServer", &iName));
	m_pHiphen = (_HiphenServer*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pHiphen, iName + " not found");

	return true;
}

bool _HiphenRGB::start(void)
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

void _HiphenRGB::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		take();

		this->autoFPSto();
	}
}

int _HiphenRGB::check(void)
{
	NULL__(m_pGPS,-1);
	NULL__(m_pCam,-1);
	NULL__(m_pHiphen,-1);

	return 0;
}

void _HiphenRGB::take(void)
{
	IF_(check()<0);

	IF_(m_pHiphen->getImgSetIdx() <= m_iImg);
	m_iImg++;

	LL_POS pLL = m_pGPS->getLLpos();
	string lat = lf2str(pLL.m_lat, 7);
	string lon = lf2str(pLL.m_lng, 7);
	string alt = lf2str(pLL.m_altRel, 3);

	Frame fBGR = *m_pCam->BGR();
	if(m_bFlip)fBGR = fBGR.flip(-1);
	Mat mBGR;
	fBGR.m()->copyTo(mBGR);
	IF_(mBGR.empty());

	IF_(mBGR.cols*mBGR.rows < m_pCam->m_w*m_pCam->m_h);

	string fName = m_pHiphen->getDir() + i2str(m_iImg) + "_rgb" + ".tiff";
	cv::imwrite(fName, mBGR, m_compress);
	string cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
	system(cmd.c_str());

	LOG_I("RGB: " + fName);
}

bool _HiphenRGB::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	string msg;

	return true;
}

bool _HiphenRGB::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	C_MSG("iImg: "+i2str(m_iImg));
	return true;
}

}
