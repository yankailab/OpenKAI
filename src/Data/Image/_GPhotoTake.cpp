#include "_GPhotoTake.h"

namespace kai
{

	_GPhotoTake::_GPhotoTake()
	{
		m_pG = nullptr;

		m_dir = "/home/";
		m_subDir = "";

		m_iTake = 0;
		m_tDelay = 0;

		m_quality = 100;
		m_bFlipRGB = false;
		m_bFlipD = false;
	}

	_GPhotoTake::~_GPhotoTake()
	{
	}

	int _GPhotoTake::init(void *pKiss)
	{
		CHECK_(this->_FileBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("quality", &m_quality);
		pK->v("dir", &m_dir);
		pK->v("subDir", &m_subDir);
		pK->v("bFlipRGB", &m_bFlipRGB);
		pK->v("bFlipD", &m_bFlipD);
		pK->v("tDelay", &m_tDelay);

		if (m_subDir.empty())
			m_subDir = m_dir + tFormat() + "/";
		else
			m_subDir = m_dir + m_subDir;

		m_compress.push_back(IMWRITE_JPEG_QUALITY);
		m_compress.push_back(m_quality);

		return OK_OK;
	}

	int _GPhotoTake::link(void)
	{
		CHECK_(this->_FileBase::link());
		CHECK_(m_pT->link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_GPhoto", &n);
		m_pG = (_GPhoto *)(pK->findModule(n));

		return OK_OK;
	}

	int _GPhotoTake::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _GPhotoTake::check(void)
	{
		NULL__(m_pG, OK_ERR_NULLPTR);

		return this->_FileBase::check();
	}

	void _GPhotoTake::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

		}
	}

	void _GPhotoTake::updateTake(void)
	{
	}

	bool _GPhotoTake::startAutoMode(int nTake, int tInterval)
	{
		IF_F(check() != OK_OK);
	}
	void _GPhotoTake::stopAutoMode(void)
	{
	}

	bool _GPhotoTake::shutter(void)
	{
		IF_F(check() != OK_OK);

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
		//		vP = m_pAP->getGlobalPos();
		string lat = lf2str(vP.x, 7);
		string lon = lf2str(vP.y, 7);
		string alt = lf2str(vP.z, 3);

		string fName;

		// gphoto
		// m_pG->open();

		// fName = m_subDir + i2str(m_iTake) + "_" + tFormat() + "_" + f2str(m_alt) + ".jpg";
		// cmd = "gphoto2 --capture-image-and-download --filename " + fName;
		// system(cmd.c_str());
		// cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
		// system(cmd.c_str());

		// LOG_I("GPhoto: " + fName);

		LOG_I("Take: " + i2str(m_iTake));
		m_iTake++;

		return true;
	}

	void _GPhotoTake::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_FileBase::console(pConsole);
		IF_(check() != OK_OK);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("iTake = " + i2str(m_iTake));
		pC->addMsg("Dir = " + m_subDir);
	}

}
