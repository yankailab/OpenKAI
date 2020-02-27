#include "_AP_shutter.h"

namespace kai
{

_AP_shutter::_AP_shutter()
{
	m_pAP = NULL;
	m_pV = NULL;
	m_pDV = NULL;
	m_pG = NULL;
	m_pDet = NULL;

	m_dir = "/home/";
	m_subDir = "";

	m_quality = 100;
	m_iTake = 0;
	m_bFlipRGB = false;
	m_bFlipD = false;

	m_shutterMode = apShutter_det;
	m_ieShutter.init(USEC_1SEC);
	m_iTag = -1;
	m_bModeChange = true;
	m_tDelay = USEC_1SEC;
	m_apModeShutter = AP_ROVER_HOLD;

}

_AP_shutter::~_AP_shutter()
{
}

bool _AP_shutter::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("quality", &m_quality);
	pK->v("dir", &m_dir);
	pK->v("subDir", &m_subDir);
	pK->v("bFlipRGB", &m_bFlipRGB);
	pK->v("bFlipD", &m_bFlipD);
	pK->v("bModeChange", &m_bModeChange);
	pK->v("tDelay", &m_tDelay);
	pK->v("apModeShutter", &m_apModeShutter);
	pK->v("tInterval", &m_ieShutter.m_tInterval);

	if(m_subDir.empty())
		m_subDir = m_dir + tFormat() + "/";
	else
		m_subDir = m_dir + m_subDir;

	m_compress.push_back(IMWRITE_JPEG_QUALITY);
	m_compress.push_back(m_quality);

	string iName;

	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->parent()->getChildInst(iName));

	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("_DepthVisionBase", &iName);
	m_pDV = (_DepthVisionBase*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("_GPhoto", &iName);
	m_pG = (_GPhoto*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("_DetectorBase", &iName);
	m_pDet = (_DetectorBase*) (pK->root()->getChildInst(iName));

	return true;
}

bool _AP_shutter::start(void)
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

int _AP_shutter::check(void)
{
	NULL__(m_pDet, -1);

	return this->_AutopilotBase::check();
}

void _AP_shutter::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();
		this->_AutopilotBase::update();

		shutter();

		this->autoFPSto();
	}
}

void _AP_shutter::shutter(void)
{
	IF_(check()<0);
	IF_(!bActive());

	if(m_shutterMode == apShutter_det)
	{
		OBJECT* pO = m_pDet->at(0);
		NULL_(pO);
		IF_(pO->m_topClass == m_iTag);
		m_iTag = pO->m_topClass;

	}
	else if(m_shutterMode == apShutter_cont)
	{
		IF_(m_ieShutter.update(m_tStamp));
	}
	else if(m_shutterMode == apShutter_manual)
	{
		NULL_(m_pAP);
		//TODO: read rc in from AP
	}


	string cmd;

	cmd = "mkdir /media/usb";
	system(cmd.c_str());

	cmd = "mount /dev/sda1 /media/usb";
	system(cmd.c_str());

	cmd = "mkdir " + m_subDir;
	system(cmd.c_str());


	vDouble4 vP;
	vP.init();
	string lat = "";
	string lon = "";
	string alt = "";
	uint32_t apModeResume;

	if(m_pAP)
	{
		apModeResume = m_pAP->getApMode();
		if(m_bModeChange)
		{
			while(m_pAP->getApMode()!=m_apModeShutter)
			{
				m_pAP->setApMode(m_apModeShutter);
				this->sleepTime(200000);
			}
			this->sleepTime(m_tDelay);
		}

		vP = m_pAP->getGlobalPos();
		lat = lf2str(vP.x, 7);
		lon = lf2str(vP.y, 7);
		alt = lf2str(vP.z, 3);
	}


	string fName;

	if(m_pV)
	{
		//rgb
		Frame fBGR = *m_pV->BGR();
		if(m_bFlipRGB)fBGR = fBGR.flip(-1);
		Mat mBGR;
		fBGR.m()->copyTo(mBGR);
		IF_(mBGR.empty());

		fName = m_subDir + i2str(m_iTake) + "_tag" + i2str(m_iTag) + "_rgb.jpg";
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
		IF_(mD.empty());

		Mat mDscale;
		mD.convertTo(mDscale, CV_8UC1, 100);

		fName = m_subDir + i2str(m_iTake)  + "_tag" + i2str(m_iTag) +  "_d.jpg";
		cv::imwrite(fName, mDscale, m_compress);
		cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
		system(cmd.c_str());

		LOG_I("Depth: " + fName);
	}

	if(m_pG)
	{
		//gphoto
		m_pG->open();

		fName = m_subDir + i2str(m_iTake)  + "_tag" + i2str(m_iTag) +  ".jpg";
		cmd = "gphoto2 --capture-image-and-download --filename " + fName;
		system(cmd.c_str());
		cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
		system(cmd.c_str());

		LOG_I("GPhoto: " + fName);
	}

	LOG_I("Take: " + i2str(m_iTake) + ", Tag: " + i2str(m_iTag));
	m_iTake++;


	if(m_pAP)
	{
		if(m_bModeChange)
			m_pAP->setApMode(apModeResume);
	}
}

void _AP_shutter::draw(void)
{
	this->_AutopilotBase::draw();
	IF_(check()<0);

	if(!bActive())
		addMsg("Inactive");

	addMsg("iTake = " + i2str(m_iTake));
	addMsg("Dir = " + m_subDir);
}

}
