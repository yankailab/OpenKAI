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
	m_classFlag = 0;
	m_vRoi.init();
	m_vRoi.z = 1.0;
	m_vRoi.w = 1.0;

	m_bTarget = false;
	m_bAlpr = true;
	m_bGDupload = true;

	m_tempDir = "GDcam_";
	m_alprAPI = "https://api.openalpr.com/v2/recognize?recognize_vehicle=0&country=jp&secret_key=";
	m_alprKey = "";
	m_gdUpload = "python gdUpload.py";
	m_gdImgFolderID = "";
	m_gdDataFolderID = "";
	m_gdCredentials = "credentials.json";

}

_GDcam::~_GDcam()
{
}

bool _GDcam::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("tempDir",&m_tempDir);
	pK->v("alprAPI",&m_alprAPI);
	pK->v("alprKey",&m_alprKey);
	pK->v("gdUpload",&m_gdUpload);
	pK->v("gdImgFolderID",&m_gdImgFolderID);
	pK->v("gdDataFolderID",&m_gdDataFolderID);
	pK->v("gdCredentials",&m_gdCredentials);
	pK->v("bAlpr",&m_bAlpr);
	pK->v("bGDupload",&m_bGDupload);

	m_classFlag = 0;
	vector<int> vClass;
	pK->a("vClass", &vClass);
	for (int i = 0; i < vClass.size(); i++)
		m_classFlag |= (1 << vClass[i]);

	pK->v("x",&m_vRoi.x);
	pK->v("y",&m_vRoi.y);
	pK->v("z",&m_vRoi.z);
	pK->v("w",&m_vRoi.w);

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

		if(findTarget())
		{
			if(!m_bTarget)
			{
				updateShot();
				m_bTarget = true;
			}
		}
		else
		{
			m_bTarget = false;
		}

		this->autoFPSto();
	}
}

int _GDcam::check(void)
{
	NULL__(m_pD, -1);
	NULL__(m_pD->m_pV, -1);
	IF__(m_pD->m_pV->BGR()->bEmpty(), -1);

	return 0;
}

bool _GDcam::findTarget(void)
{
	IF_F(check() < 0);

	OBJECT* pO;
	int i = 0;
	while ((pO = m_pD->at(i++)) != NULL)
	{
		IF_CONT(!(m_classFlag & (1 << pO->m_topClass)));
		IF_CONT(pO->m_bb.x < m_vRoi.x);
		IF_CONT(pO->m_bb.x > m_vRoi.z);
		IF_CONT(pO->m_bb.y < m_vRoi.y);
		IF_CONT(pO->m_bb.y > m_vRoi.w);

		return true;
	}

	return false;
}

void _GDcam::oAlpr(const string& fName)
{
	FILE *fp;
	char path[1035];
	string alpr = "curl -X POST -F image=@" + m_tempDir + fName + ".jpeg '" + m_alprAPI + m_alprKey + "'";
	fp = popen(alpr.c_str(), "r");
	IF_l(!fp, "Failed to run command:" + alpr);

	while (fgets(path, sizeof(path) - 1, fp));
	pclose(fp);
	string strR = string(path);

	string file = m_tempDir + fName + ".txt";
	m_fAlpr.open(file.c_str(), ios::out);
	IF_(!m_fAlpr.is_open());
	m_fAlpr.seekg(0, ios_base::beg);

	IF_(!m_fAlpr.write((char*)strR.c_str(), strR.length()));

	m_fAlpr.flush();
	m_fAlpr.close();
}

void _GDcam::gdUpload(const string& fName)
{
	//Upload to Google Drive
	string cmd = m_gdUpload + " " + m_tempDir + fName + ".jpeg "
			+ fName + ".jpg image/jpeg "
			+ m_gdImgFolderID
			+ " " + m_gdCredentials;
	system(cmd.c_str());

	cmd = m_gdUpload + " " + m_tempDir + fName + ".txt "
			+ fName + ".txt text/plain "
			+ m_gdDataFolderID
			+ " " + m_gdCredentials;
	system(cmd.c_str());

	cmd = "rm " + m_tempDir + fName + ".jpeg";
	system(cmd.c_str());

	cmd = "rm " + m_tempDir + fName + ".txt";
	system(cmd.c_str());
}

void _GDcam::updateShot(void)
{
	IF_(check() < 0);

	m_fBGR.copy(*m_pD->m_pV->BGR());
	string fImg = tFormat();
	cv::imwrite(m_tempDir + fImg + ".jpeg", *m_fBGR.m(), m_vJPGquality);

	if(m_bAlpr)
		oAlpr(fImg);

	if(m_bGDupload)
		gdUpload(fImg);

}

void _GDcam::draw(void)
{
	this->_ThreadBase::draw();

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Scalar col = Scalar(0,255,0);

	Rect r = convertBB<vInt4>(convertBB(m_vRoi, cs));
	rectangle(*pMat, r, col, 3);
}

}
