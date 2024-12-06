/*
 * _GDimgUploader.cpp
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#include "_GDimgUploader.h"

namespace kai
{

	_GDimgUploader::_GDimgUploader()
	{
		m_pV = nullptr;
		m_tInterval = SEC_2_USEC;
		m_tLastUpload = 0;
		m_tempDir = "GDcam_";
		m_gdUpload = "python gdUpload.py";
		m_gdFolderID = "";
		m_gdCredentials = "credentials.json";
	}

	_GDimgUploader::~_GDimgUploader()
	{
	}

	int _GDimgUploader::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("tInterval", &m_tInterval);
		pK->v("tempDir", &m_tempDir);
		pK->v("gdUpload", &m_gdUpload);
		pK->v("gdFolderID", &m_gdFolderID);
		pK->v("gdCredentials", &m_gdCredentials);

		int jpgQuality = 80;
		pK->v("jpgQuality", &jpgQuality);
		m_vJPGquality.push_back(IMWRITE_JPEG_QUALITY);
		m_vJPGquality.push_back(jpgQuality);

		string n;

		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _GDimgUploader::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _GDimgUploader::check(void)
	{
		NULL__(m_pV, OK_ERR_NULLPTR);
		IF__(m_pV->getFrameRGB()->bEmpty(), OK_ERR_NOT_READY);

		return OK_OK;
	}

	void _GDimgUploader::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			if (m_pT->getTfrom() - m_tLastUpload > m_tInterval)
			{
				updateUpload();
				m_tLastUpload = m_pT->getTfrom();
			}

		}
	}

	void _GDimgUploader::updateUpload(void)
	{
		IF_(check() != OK_OK);

		Mat m;
		m_pV->getFrameRGB()->m()->copyTo(m);

		//save img
		string fImg = tFormat();
		cv::imwrite(m_tempDir + fImg + ".jpeg", m, m_vJPGquality);

		//upload img to Google Drive
		string cmd = m_gdUpload + " " + m_tempDir + fImg + ".jpeg " + fImg + ".jpg image/jpeg " + m_gdFolderID + " " + m_gdCredentials;
		system(cmd.c_str());

		cmd = "rm " + m_tempDir + fImg + ".jpeg";
		system(cmd.c_str());
	}

}
