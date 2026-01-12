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

	int _APmavlink_photo::init(const json& j)
	{
		CHECK_(this->_ModuleBase::init(j));

		= j.value("exifConfig", &m_exifConfig);
		= j.value("dir", &m_dir);
		= j.value("subDir", &m_subDir);

		if (m_subDir.empty())
			m_subDir = m_dir + tFormat() + "/";
		else
			m_subDir = m_dir + m_subDir;

		return true;
	}

	int _APmavlink_photo::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_ModuleBase::link(j, pM));

		string n;

		n = "";
		= j.value("_APmavlink_base", &n);
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL__(m_pAP);

		return true;
	}

	int _APmavlink_photo::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_photo::check(void)
	{
		NULL__(m_pAP);

		return this->_ModuleBase::check();
	}

	void _APmavlink_photo::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updatePhoto();

		}
	}

	void _APmavlink_photo::updatePhoto(void)
	{
		IF_(!check());

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
			  " -overwrite_original" +
			  " -GPSLongitude=" + lon +
			  " -GPSLatitude=" + lat +
			  " -GPSAltitude=" + alt +
			  " -GPSImgDirection=" + yaw +
			  " -GPSPitch=" + pitch +
			  " -GPSRoll=" + roll + " " +
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
		IF_(!check());

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("iTake = " + i2str(m_iTake));
		pC->addMsg("Dir = " + m_subDir);
	}

}
