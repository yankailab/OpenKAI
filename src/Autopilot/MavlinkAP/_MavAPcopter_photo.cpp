#include "_MavAPcopter_photo.h"

namespace kai
{

	_MavAPcopter_photo::_MavAPcopter_photo()
	{
		m_pAP = nullptr;
		m_pPC = nullptr;
		m_pDS = nullptr;
		m_pV = nullptr;
		m_pDV = nullptr;
		m_pG = nullptr;

		m_iDiv = 0;
		m_speed = 0.3;
		m_dAlt = 1.0;
		m_alt = 0.0;
		m_lastAlt = 0.0;
		m_iRelayShutter = 1;
		m_iRCshutter = 7;
		m_scShutter.init(0);

		m_dir = "/home/";
		m_subDir = "";

		m_iTake = 0;
		m_tDelay = 0;

		m_quality = 100;
		m_bFlipRGB = false;
		m_bFlipD = false;
	}

	_MavAPcopter_photo::~_MavAPcopter_photo()
	{
	}

	int _MavAPcopter_photo::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("quality", &m_quality);
		pK->v("dir", &m_dir);
		pK->v("subDir", &m_subDir);
		pK->v("bFlipRGB", &m_bFlipRGB);
		pK->v("bFlipD", &m_bFlipD);
		pK->v("tDelay", &m_tDelay);
		pK->v("iDiv", &m_iDiv);
		pK->v("dAlt", &m_dAlt);
		pK->v("speed", &m_speed);
		pK->v("iRelayShutter", &m_iRelayShutter);
		pK->v("iRCshutter", &m_iRCshutter);

		if (m_subDir.empty())
			m_subDir = m_dir + tFormat() + "/";
		else
			m_subDir = m_dir + m_subDir;

		m_compress.push_back(IMWRITE_JPEG_QUALITY);
		m_compress.push_back(m_quality);

		string n;

		n = "";
		pK->v("_MavAP_base", &n);
		m_pAP = (_MavAP_base *)(pK->findModule(n));
		NULL__(m_pAP, OK_ERR_NOT_FOUND);

		n = "";
		pK->v("_MavAP_move", &n);
		m_pPC = (_MavAP_move *)(pK->findModule(n));
		NULL__(m_pPC, OK_ERR_NOT_FOUND);

		n = "";
		pK->v("_DistSensorBase", &n);
		m_pDS = (_DistSensorBase *)(pK->findModule(n));
		NULL__(m_pDS, OK_ERR_NOT_FOUND);

		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));

		n = "";
		pK->v("_RGBDbase", &n);
		m_pDV = (_RGBDbase *)(pK->findModule(n));

		n = "";
		pK->v("_GPhoto", &n);
		m_pG = (_GPhoto *)(pK->findModule(n));

		return OK_OK;
	}

	int _MavAPcopter_photo::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _MavAPcopter_photo::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);
		NULL__(m_pPC, OK_ERR_NULLPTR);
		NULL__(m_pDS, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _MavAPcopter_photo::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			if (check() >= 0)
			{
				m_pAP->m_pMav->clDoSetRelay(m_iRelayShutter, false);

				int apMode = m_pAP->getMode();
				if (apMode == AP_COPTER_GUIDED)
				{
					autoMode();
				}
				else
				{
					m_lastAlt = 0.0;
					manualMode();
				}
			}

			m_pT->autoFPSto();
		}
	}

	void _MavAPcopter_photo::autoMode(void)
	{
		IF_(check() != OK_OK);

		// m_alt = m_pDS->d(m_iDiv);
		// IF_(m_alt < 0.0);

		// if (m_alt - m_lastAlt <= m_dAlt)
		// {
		// 	m_pPC->m_vPvar.z = m_speed;
		// 	return;
		// }

		// m_lastAlt = floorf(m_alt);
		// if (m_lastAlt < 0.0)
		// 	m_lastAlt = 0.0;
		// m_pPC->m_vPvar.z = 0.0;

		shutter();
	}

	void _MavAPcopter_photo::manualMode(void)
	{
		IF_(check() != OK_OK);

		uint16_t rcShutter = m_pAP->m_pMav->m_rcChannels.getRC(m_iRCshutter);
		IF_(rcShutter == UINT16_MAX);

		m_scShutter.update((rcShutter > 1750) ? 1 : 0);
		if (m_scShutter.bActive(1))
			shutter();
	}

	void _MavAPcopter_photo::shutter(void)
	{
		IF_(check() != OK_OK);

		m_pAP->m_pMav->clDoSetRelay(m_iRelayShutter, true);

		if (m_tDelay > 0)
			m_pT->sleepT(m_tDelay);

		string cmd;
		cmd = "mkdir /media/usb";
		system(cmd.c_str());
		cmd = "mount /dev/sda1 /media/usb";
		system(cmd.c_str());
		cmd = "mkdir " + m_subDir;
		system(cmd.c_str());

		vDouble4 vP;
		vP.clear();
		vP = m_pAP->getGlobalPos();
		string lat = lf2str(vP.x, 7);
		string lon = lf2str(vP.y, 7);
		string alt = lf2str(vP.z, 3);

		string fName;

		if (m_pV)
		{
			//rgb
			Frame fBGR = *m_pV->getFrameRGB();
			if (m_bFlipRGB)
				fBGR = fBGR.flip(-1);
			Mat mBGR;
			fBGR.m()->copyTo(mBGR);
			IF_(mBGR.empty());

			fName = m_subDir + i2str(m_iTake) + "_rgb.jpg";
			cv::imwrite(fName, mBGR, m_compress);
			cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
			system(cmd.c_str());

			LOG_I("RGB: " + fName);
		}

		if (m_pDV)
		{
			//depth
			Frame fD = *m_pDV->getFrameD();
			if (m_bFlipD)
				fD = fD.flip(-1);
			Mat mD;
			fD.m()->copyTo(mD);
			IF_(mD.empty());

			Mat mDscale;
			mD.convertTo(mDscale, CV_8UC1, 100);

			fName = m_subDir + i2str(m_iTake) + "_d.jpg";
			cv::imwrite(fName, mDscale, m_compress);
			cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
			system(cmd.c_str());

			LOG_I("Depth: " + fName);
		}

		if (m_pG)
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

		m_pAP->m_pMav->clDoSetRelay(m_iRelayShutter, false);
	}

	void _MavAPcopter_photo::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(check() != OK_OK);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("alt = " + f2str(m_alt) + ", lastAlt = " + f2str(m_lastAlt) + ", dAlt = " + f2str(m_dAlt));
		pC->addMsg("iTake = " + i2str(m_iTake));
		pC->addMsg("Dir = " + m_subDir);
	}

}
