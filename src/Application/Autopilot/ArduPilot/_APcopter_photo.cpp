#include "_APcopter_photo.h"

namespace kai
{

_APcopter_photo::_APcopter_photo()
{
	m_pAP = NULL;
	m_pPC = NULL;
	m_pDS = NULL;
	m_pV = NULL;
	m_pDV = NULL;
	m_pG = NULL;

	m_iDiv = 0;
	m_speed = 0.3;
	m_dAlt = 1.0;
	m_alt = 0.0;
	m_lastAlt = 0.0;
	m_iRelayLED = 0;
	m_iRCmode = 7;

	m_dir = "/home/";
	m_subDir = "";

	m_iTake = 0;
	m_tDelay = 0;

	m_quality = 100;
	m_bFlipRGB = false;
	m_bFlipD = false;

}

_APcopter_photo::~_APcopter_photo()
{
}

bool _APcopter_photo::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("quality", &m_quality);
	pK->v("dir", &m_dir);
	pK->v("subDir", &m_subDir);
	pK->v("bFlipRGB", &m_bFlipRGB);
	pK->v("bFlipD", &m_bFlipD);
	pK->v("tDelay", &m_tDelay);
	pK->v("iDiv", &m_iDiv);
	pK->v("dAlt", &m_dAlt);
	pK->v("speed", &m_speed);
	pK->v("iRelayLED",&m_iRelayLED);
	pK->v("iRCmode",&m_iRCmode);

	if(m_subDir.empty())
		m_subDir = m_dir + tFormat() + "/";
	else
		m_subDir = m_dir + m_subDir;

	m_compress.push_back(IMWRITE_JPEG_QUALITY);
	m_compress.push_back(m_quality);

	string iName;

	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pAP, iName + ": not found");

	iName = "";
	pK->v("_AP_posCtrl", &iName);
	m_pPC = (_AP_posCtrl*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pPC, iName + ": not found");

	iName = "";
	pK->v("_DistSensorBase", &iName);
	m_pDS = (_DistSensorBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pDS, iName + ": not found");

	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("_DepthVisionBase", &iName);
	m_pDV = (_DepthVisionBase*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("_GPhoto", &iName);
	m_pG = (_GPhoto*) (pK->root()->getChildInst(iName));

	return true;
}

bool _APcopter_photo::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: " << retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _APcopter_photo::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pPC, -1);
	NULL__(m_pDS, -1);

	return this->_AutopilotBase::check();
}

void _APcopter_photo::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();
		this->_AutopilotBase::update();

		if(!shutter())
		{
			m_lastAlt = 0.0;
		}

		this->autoFPSto();
	}
}

bool _APcopter_photo::shutter(void)
{
	IF_F(check()<0);

//	uint16_t rcMode = m_pAP->m_pMav->m_rcChannels.getRC(m_iRCmode);
//	IF_F(rcMode == UINT16_MAX);
//	//mode
//	if(rcMode < 1250)
//	else if(rcMode > 1750)

	IF_F(m_pAP->getApMode() != AP_COPTER_GUIDED);

	m_alt = m_pDS->d(m_iDiv);
	IF_T(m_alt < 0.0);

	if(m_alt - m_lastAlt <= m_dAlt)
	{
		m_pPC->m_vP.z = m_speed;
		return true;
	}

	m_pAP->m_pMav->clDoSetRelay(m_iRelayLED, 1);

	m_lastAlt = floorf(m_alt);
	if(m_lastAlt<0.0)m_lastAlt=0.0;
	m_pPC->m_vP.z = 0.0;

	if(m_tDelay > 0)
		this->sleepTime(m_tDelay);

	string cmd;
	cmd = "mkdir /media/usb";
	system(cmd.c_str());
	cmd = "mount /dev/sda1 /media/usb";
	system(cmd.c_str());
	cmd = "mkdir " + m_subDir;
	system(cmd.c_str());

	vDouble4 vP;
	vP.init();
	vP = m_pAP->getGlobalPos();
	string lat = lf2str(vP.x, 7);
	string lon = lf2str(vP.y, 7);
	string alt = lf2str(vP.z, 3);

	string fName;

	if(m_pV)
	{
		//rgb
		Frame fBGR = *m_pV->BGR();
		if(m_bFlipRGB)fBGR = fBGR.flip(-1);
		Mat mBGR;
		fBGR.m()->copyTo(mBGR);
		IF_F(mBGR.empty());

		fName = m_subDir + i2str(m_iTake) + "_rgb.jpg";
		cv::imwrite(fName, mBGR, m_compress);
		cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
		system(cmd.c_str());

		LOG_I("RGB: " + fName);
	}

	if(m_pDV)
	{
		//depth
		Frame fD = *m_pDV->Depth();
		if(m_bFlipD)fD = fD.flip(-1);
		Mat mD;
		fD.m()->copyTo(mD);
		IF_F(mD.empty());

		Mat mDscale;
		mD.convertTo(mDscale, CV_8UC1, 100);

		fName = m_subDir + i2str(m_iTake) + "_d.jpg";
		cv::imwrite(fName, mDscale, m_compress);
		cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
		system(cmd.c_str());

		LOG_I("Depth: " + fName);
	}

	if(m_pG)
	{
		//gphoto
		m_pG->open();

		fName = m_subDir + i2str(m_iTake) + "_" + tFormat() + "_" + f2str(m_alt) + ".jpg";
		cmd = "gphoto2 --capture-image-and-download --filename " + fName;
		system(cmd.c_str());
		cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
		system(cmd.c_str());

		LOG_I("GPhoto: " + fName);
	}

	LOG_I("Take: " + i2str(m_iTake));
	m_iTake++;

	m_pAP->m_pMav->clDoSetRelay(m_iRelayLED, 0);

	return true;
}

void _APcopter_photo::draw(void)
{
	IF_(check()<0);
	this->_AutopilotBase::draw();
	drawActive();

	addMsg("alt = " + f2str(m_alt) + ", lastAlt = " + f2str(m_lastAlt) + ", dAlt = " + f2str(m_dAlt));
	addMsg("iTake = " + i2str(m_iTake));
	addMsg("Dir = " + m_subDir);
}

}
