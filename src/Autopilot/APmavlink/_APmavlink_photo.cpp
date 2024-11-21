#include "_APmavlink_photo.h"

namespace kai
{

	_APmavlink_photo::_APmavlink_photo()
	{
		m_pAP = nullptr;
		m_dir = "/home/";
		m_subDir = "";
		m_exifConfig = "";
		m_iTake = 0;
	}

	_APmavlink_photo::~_APmavlink_photo()
	{
	}

	int _APmavlink_photo::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("exifConfig", &m_exifConfig);
		pK->v("dir", &m_dir);
		pK->v("subDir", &m_subDir);

		if (m_subDir.empty())
			m_subDir = m_dir + tFormat() + "/";
		else
			m_subDir = m_dir + m_subDir;

		return OK_OK;
	}

	int _APmavlink_photo::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;

		n = "";
		pK->v("_APmavlink_base", &n);
		m_pAP = (_APmavlink_base *)(pK->findModule(n));
		NULL__(m_pAP, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _APmavlink_photo::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_photo::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _APmavlink_photo::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updatePhoto();

			m_pT->autoFPSto();
		}
	}

	void _APmavlink_photo::updatePhoto(void)
	{
		IF_(check() != OK_OK);

		string cmd;
		// cmd = "mkdir /media/usb";
		// system(cmd.c_str());
		// cmd = "mount /dev/sda1 /media/usb";
		// system(cmd.c_str());
		cmd = "mkdir " + m_subDir;
		system(cmd.c_str());

		vDouble4 vP;
		vP = m_pAP->getGlobalPos();
		string lat = lf2str(vP.x, 7);
		string lon = lf2str(vP.y, 7);
		string alt = lf2str(vP.z, 3);
		string altRel = lf2str(vP.w, 3);

		vFloat3 vA;
		vA = m_pAP->getAttitude();
		//		string yaw = f2str(vA.x * RAD_2_DEG, 3);
		string pitch = f2str(vA.y * RAD_2_DEG, 3);
		string roll = f2str(vA.z * RAD_2_DEG, 3);

		string yaw = f2str(m_pAP->getHdg(), 3);

		string fName;

		// gphoto
		// m_cmdUnmount = "gio mount -s gphoto2";
		// system(m_cmdUnmount.c_str());

		fName = m_subDir + i2str(m_iTake) + "_" + tFormat() + "_" + ".jpg";
		cmd = "gphoto2 --capture-image-and-download --filename " + fName;
		system(cmd.c_str());
		cmd = "exiftool " + m_exifConfig +
			  " -overwrite_original " +
			  " -GPSLongitude=" + lon +
			  " -GPSLatitude=" + lat +
			  " -GPSAltitude=" + alt +
			  " -GPSImgDirection=" + yaw +
			  " -GPSPitch=" + pitch +
			  " -GPSRoll=" + roll +
			  fName;
		system(cmd.c_str());
		LOG_I("GPhoto: " + fName);

		LOG_I("Take: " + i2str(m_iTake));
		m_iTake++;

		// if (m_pV)
		// {
		// 	//rgb
		// 	Frame fBGR = *m_pV->getFrameRGB();
		// 	if (m_bFlipRGB)
		// 		fBGR = fBGR.flip(-1);
		// 	Mat mBGR;
		// 	fBGR.m()->copyTo(mBGR);
		// 	IF_(mBGR.empty());

		// 	fName = m_subDir + i2str(m_iTake) + "_rgb.jpg";
		// 	cv::imwrite(fName, mBGR, m_compress);
		// 	cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
		// 	system(cmd.c_str());

		// 	LOG_I("RGB: " + fName);
		// }
	}

	void _APmavlink_photo::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(check() != OK_OK);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("iTake = " + i2str(m_iTake));
		pC->addMsg("Dir = " + m_subDir);
	}

}
