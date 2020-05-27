/*
 * _GDimgUploader.cpp
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#include "_GDimgUploader.h"

#ifdef USE_OPENCV

namespace kai
{

_GDimgUploader::_GDimgUploader()
{
	m_pV = NULL;
	m_tInterval = USEC_1SEC;
	m_tLastUpload = 0;
	m_tempDir = "GDcam_";
	m_gdUpload = "python gdUpload.py";
	m_gdFolderID = "";
	m_gdCredentials = "credentials.json";

}

_GDimgUploader::~_GDimgUploader()
{
}

bool _GDimgUploader::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("tInterval",&m_tInterval);
	pK->v("tempDir",&m_tempDir);
	pK->v("gdUpload",&m_gdUpload);
	pK->v("gdFolderID",&m_gdFolderID);
	pK->v("gdCredentials",&m_gdCredentials);

	int jpgQuality = 80;
	pK->v("jpgQuality", &jpgQuality);
	m_vJPGquality.push_back(IMWRITE_JPEG_QUALITY);
	m_vJPGquality.push_back(jpgQuality);

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_VisionBase", &iName));
	m_pV = (_VisionBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pV, iName + " not found");

	return true;
}

bool _GDimgUploader::start(void)
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

void _GDimgUploader::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		if(m_tStamp - m_tLastUpload > m_tInterval)
		{
			updateUpload();
			m_tLastUpload = m_tStamp;
		}

		this->autoFPSto();
	}
}

int _GDimgUploader::check(void)
{
	NULL__(m_pV, -1);
	IF__(m_pV->BGR()->bEmpty(), -1);

	return 0;
}

void _GDimgUploader::updateUpload(void)
{
	IF_(check() < 0);

	Mat m;
	m_pV->BGR()->m()->copyTo(m);

	//save img
	string fImg = tFormat();
	cv::imwrite(m_tempDir + fImg + ".jpeg", m, m_vJPGquality);

	//upload img to Google Drive
	string cmd = m_gdUpload + " " + m_tempDir + fImg + ".jpeg "
			+ fImg + ".jpg image/jpeg "
			+ m_gdFolderID
			+ " " + m_gdCredentials;
	system(cmd.c_str());

	cmd = "rm " + m_tempDir + fImg + ".jpeg";
	system(cmd.c_str());
}

void _GDimgUploader::draw(void)
{
	this->_ThreadBase::draw();
}

}
#endif
