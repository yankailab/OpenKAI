#include "APcopter_takePhoto.h"

namespace kai
{

APcopter_takePhoto::APcopter_takePhoto()
{
	m_pAP = NULL;
	m_pV = NULL;
	m_pDV = NULL;

	m_dir = "/home/";
	m_subDir = "";

	m_tInterval = USEC_1SEC * 3;
	m_tLastTake = 0;
	m_quality = 100;
	m_iTake = 0;
}

APcopter_takePhoto::~APcopter_takePhoto()
{
}

bool APcopter_takePhoto::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,quality);
	KISSm(pK,dir);
	KISSm(pK,subDir);
	if(pK->v("tInterval",&m_tInterval))
	{
		m_tInterval *= USEC_1SEC;
	}

	if(m_subDir.empty())
	{
		m_subDir = m_dir + tFormat() + "/";
	}
	else
	{
		m_subDir = m_dir + m_subDir;
	}

	string cmd = "mkdir " + m_subDir;
	system(cmd.c_str());

	m_compress.push_back(CV_IMWRITE_JPEG_QUALITY);
	m_compress.push_back(m_quality);

	//link
	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));

	iName = "";
	F_INFO(pK->v("_VisionBase", &iName));
	m_pV = (_VisionBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pV, iName + " not found");

	iName = "";
	F_INFO(pK->v("_DepthVisionBase", &iName));
	m_pDV = (_DepthVisionBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDV, iName + " not found");

	return true;
}

void APcopter_takePhoto::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	NULL_(m_pV);
	IF_(!m_pV->m_bOpen);
	NULL_(m_pDV);
	IF_(!m_pDV->m_bOpen);

	IF_(m_tStamp - m_tLastTake < m_tInterval);
	m_tLastTake = m_tStamp;

	Mat mRGB;
	m_pV->BGR()->m()->copyTo(mRGB);
	IF_(mRGB.empty());

	Mat mD;
	m_pDV->Depth()->m()->copyTo(mD);
	IF_(mD.empty());

	string lat = f2str(m_pAP->m_pMavlink->m_msg.global_position_int.lat * 0.0000001);
	string lon = f2str(m_pAP->m_pMavlink->m_msg.global_position_int.lon * 0.0000001);
	string alt = f2str(m_pAP->m_pMavlink->m_msg.global_position_int.alt * 0.001);
	string hnd = f2str(m_pAP->m_pMavlink->m_msg.global_position_int.hdg * 0.01);

	string fName;
	string cmd;

	//rgb
	fName = m_subDir + "rgb_" + i2str(m_iTake) + ".jpg";
	cv::imwrite(fName, mRGB, m_compress);
	cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
	system(cmd.c_str());

	//depth
	fName = m_subDir + "d_" + i2str(m_iTake) + ".jpg";
	cv::imwrite(fName, mD, m_compress);
	cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
	system(cmd.c_str());

	m_iTake++;
}

}
