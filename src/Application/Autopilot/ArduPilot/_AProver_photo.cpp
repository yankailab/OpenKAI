#include "_AProver_photo.h"

namespace kai
{

_AProver_photo::_AProver_photo()
{
	m_pAP = NULL;
	m_pV = NULL;
	m_pDV = NULL;
	m_pG = NULL;
	m_pDet = NULL;

	m_pDrive = NULL;
	m_pPIDhdg = NULL;

	m_nSpeed = 1.0;
	m_dHdg = 0.0;
	m_xTarget = 0.5;

	m_dir = "/home/";
	m_subDir = "";

	m_quality = 100;
	m_iTake = 0;
	m_bFlipRGB = false;
	m_bFlipD = false;

	m_shutterMode = apShutter_det;
	m_ieShutter.init(USEC_1SEC);
	m_iTag = -1;
	m_tDelay = USEC_1SEC;
	m_vTargetBB.init();
//	m_bModeChange = false;
//	m_apModeShutter = AP_ROVER_HOLD;

}

_AProver_photo::~_AProver_photo()
{
}

bool _AProver_photo::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("quality", &m_quality);
	pK->v("dir", &m_dir);
	pK->v("subDir", &m_subDir);
	pK->v("bFlipRGB", &m_bFlipRGB);
	pK->v("bFlipD", &m_bFlipD);
	pK->v("tDelay", &m_tDelay);
	pK->v("tInterval", &m_ieShutter.m_tInterval);
	pK->v("nSpeed", &m_nSpeed);
	pK->v("xTarget", &m_xTarget);
//	pK->v("bModeChange", &m_bModeChange);
//	pK->v("apModeShutter", &m_apModeShutter);

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
	pK->v("_AProver_drive", &iName);
	m_pDrive = (_AProver_drive*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pDrive, iName + ": not found");

	iName = "";
	pK->v("PIDctrl", &iName);
	m_pPIDhdg = (PIDctrl*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pPIDhdg, iName + ": not found");

	iName = "";
	pK->v("_DetectorBase", &iName);
	m_pDet = (_DetectorBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pDet, iName + ": not found");

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

bool _AProver_photo::start(void)
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

int _AProver_photo::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pPIDhdg, -1);
	NULL__(m_pDrive, -1);
	NULL__(m_pDet, -1);

	return this->_AutopilotBase::check();
}

void _AProver_photo::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();
		this->_AutopilotBase::update();

		shutter();

		this->autoFPSto();
	}
}

void _AProver_photo::shutter(void)
{
	IF_(check()<0);
	IF_(!bActive());

	IF_(m_pAP->getApMode() != AP_ROVER_GUIDED);

	if(m_shutterMode == apShutter_det)
	{
		OBJECT* tO = NULL;
		OBJECT* pO;
		float yO = 1.0;
		int i = 0;
		while ((pO = m_pDet->at(i++)) != NULL)
		{
			IF_CONT(pO->m_bb.midY() > yO);

			tO = pO;
			yO = pO->m_bb.midY();
		}

		if(!tO)
		{
			m_vTargetBB.init();
			return;
		}

		if(tO->m_topClass == m_iTag)
		{
			m_vTargetBB.init();
			return;
		}

		m_vTargetBB = tO->m_bb;
		m_iTag = tO->m_topClass;
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


	m_pDrive->setSpeed(0.0);
	this->sleepTime(m_tDelay);

	string cmd;
	cmd = "mkdir /media/usb";
	system(cmd.c_str());
	cmd = "mount /dev/sda1 /media/usb";
	system(cmd.c_str());
	cmd = "mkdir " + m_subDir;
	system(cmd.c_str());


//	uint32_t apModeResume = m_pAP->getApMode();
//	if(m_bModeChange)
//	{
//		while(m_pAP->getApMode()!=m_apModeShutter)
//		{
//			m_pAP->setApMode(m_apModeShutter);
//			this->sleepTime(200000);
//		}
//		this->sleepTime(m_tDelay);
//	}

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


	float xTag = m_vTargetBB.midX();
	m_dHdg = m_pPIDhdg->update(xTag, m_xTarget, m_tStamp);
	m_pDrive->setYaw(m_dHdg);
	m_pDrive->setSpeed(m_nSpeed);

//	if(m_bModeChange)
//		m_pAP->setApMode(apModeResume);
}

void _AProver_photo::draw(void)
{
	IF_(check()<0);
	this->_AutopilotBase::draw();
	drawActive();

	addMsg("iTake = " + i2str(m_iTake));
	addMsg("Dir = " + m_subDir);

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Rect r = convertBB<vInt4>(convertBB(m_vTargetBB, cs));
	rectangle(*pMat, r, Scalar(0,0,255), 2);

}

}
