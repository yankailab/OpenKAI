#include "APcopter_takePhoto.h"

namespace kai
{

APcopter_takePhoto::APcopter_takePhoto()
{
	m_pAP = NULL;
	m_pDV = NULL;

	m_dir = "/home/";
	m_subDir = "";

	m_tInterval = USEC_1SEC * 3;
	m_tLastTake = 0;
	m_PNGcompression = 0;
}

APcopter_takePhoto::~APcopter_takePhoto()
{
}

bool APcopter_takePhoto::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,tInterval);
	KISSm(pK,PNGcompression);
	KISSm(pK,dir);
	KISSm(pK,subDir);

	if(m_subDir.empty())
		m_subDir = m_dir + tFormat() + "/";
	else
		m_subDir = m_dir + m_subDir;

	m_PNGcompress.push_back(CV_IMWRITE_PNG_COMPRESSION);
	m_PNGcompress.push_back(m_PNGcompression);

	//link
	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));

	iName = "";
	F_INFO(pK->v("_VisionBase", &iName));
	m_pDV = (_DepthVisionBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDV, iName + " not found");

	return true;
}

void APcopter_takePhoto::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	NULL_(m_pDV);
	IF_(!m_pDV->m_bOpen);

	IF_(m_tStamp - m_tLastTake < m_tInterval);
	m_tLastTake = m_tStamp;

	Mat mImg;
	m_pDV->BGR()->m()->copyTo(mImg);
	IF_(mImg.empty());

	string lat = f2str(m_pAP->m_pMavlink->m_msg.global_position_int.lat * 0.0000001);
	string lon = f2str(m_pAP->m_pMavlink->m_msg.global_position_int.lon * 0.0000001);
	string alt = f2str(m_pAP->m_pMavlink->m_msg.global_position_int.alt * 0.001);
	string hnd = f2str(m_pAP->m_pMavlink->m_msg.global_position_int.hdg * 0.01);

	string fName = m_subDir + "img_"
					+ lat + "_"
					+ lon + "_"
					+ alt + "_"
					+ uuid() + ".png";

	cv::imwrite(fName, mImg, m_PNGcompress);

	//TODO: write depth image, write coordinate into file property
}

}
