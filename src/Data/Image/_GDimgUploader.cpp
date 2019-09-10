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
	m_pD = NULL;

	m_tempDir = "GDcam_";
	m_gdUpload = "python gdUpload.py";
	m_gdImgFolderID = "";
	m_gdDataFolderID = "";
	m_gdCredentials = "credentials.json";

}

_GDimgUploader::~_GDimgUploader()
{
}

bool _GDimgUploader::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("tempDir",&m_tempDir);
	pK->v("gdUpload",&m_gdUpload);
	pK->v("gdImgFolderID",&m_gdImgFolderID);
	pK->v("gdDataFolderID",&m_gdDataFolderID);
	pK->v("gdCredentials",&m_gdCredentials);

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

		updateUpload();

		this->autoFPSto();
	}
}

int _GDimgUploader::check(void)
{
	NULL__(m_pD, -1);

	return 0;
}

void _GDimgUploader::updateUpload(void)
{
	IF_(check() < 0);

	//find new image
	int i = 0;
	OBJECT *pO;
	while ((pO = m_pD->at(i++)))
	{
		IF_CONT(!pO->m_bVerified);
//		IF_CONT(!pO->m_bUploaded);

		gdUpload(pO);
	}
}

void _GDimgUploader::gdUpload(OBJECT* pO)
{
	NULL_(pO);
	IF_(pO->m_mImg.empty());

	//save img
	string fImg = tFormat();
	cv::imwrite(m_tempDir + fImg + ".jpeg", pO->m_mImg, m_vJPGquality);

	//upload img to Google Drive
	string cmd = m_gdUpload + " " + m_tempDir + fImg + ".jpeg "
			+ fImg + ".jpg image/jpeg "
			+ m_gdImgFolderID
			+ " " + m_gdCredentials;
	system(cmd.c_str());

	cmd = "rm " + m_tempDir + fImg + ".jpeg";
	system(cmd.c_str());

	//save meta data
	string fMeta = m_tempDir + fImg + ".txt";
	m_fMeta.open(fMeta.c_str(), ios::out);
	IF_(!m_fMeta.is_open());
	m_fMeta.seekg(0, ios_base::beg);

	string strMeta = "";
	IF_(!m_fMeta.write((char*)strMeta.c_str(), strMeta.length()));
	m_fMeta.flush();
	m_fMeta.close();

	cmd = m_gdUpload + " " + m_tempDir + fImg + ".txt "
			+ fImg + ".txt text/plain "
			+ m_gdDataFolderID
			+ " " + m_gdCredentials;
	system(cmd.c_str());

	cmd = "rm " + fMeta;
	system(cmd.c_str());
}

bool _GDimgUploader::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

bool _GDimgUploader::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}
