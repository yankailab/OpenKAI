/*
 * _HiphenRGB.cpp
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#include "_HiphenRGB.h"

#ifdef USE_OPENCV

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
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("bFlip", &m_bFlip);
	pK->v("quality", &m_quality);
	m_compress.push_back(IMWRITE_JPEG_QUALITY);
	m_compress.push_back(m_quality);

	string n;

	n = "";
	F_ERROR_F(pK->v("_GPS", &n));
	m_pGPS = (_GPS*) (pK->getInst(n));
	IF_Fl(!m_pGPS, n + " not found");

	n = "";
	F_ERROR_F(pK->v("_Camera", &n));
	m_pCam = (_Camera*) (pK->getInst(n));
	IF_Fl(!m_pCam, n + " not found");

	n = "";
	F_ERROR_F(pK->v("_HiphenServer", &n));
	m_pHiphen = (_HiphenServer*) (pK->getInst(n));
	IF_Fl(!m_pHiphen, n + " not found");

	return true;
}

bool _HiphenRGB::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdate, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _HiphenRGB::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		take();

		m_pT->autoFPSto();
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

void _HiphenRGB::draw(void)
{
	this->_ModuleBase::draw();

	addMsg("iImg: "+i2str(m_iImg));
}

}
#endif
