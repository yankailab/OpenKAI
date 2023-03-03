#include "_PhotoTake.h"

namespace kai
{

	_PhotoTake::_PhotoTake()
	{
		m_pV = NULL;

		m_dir = "/home/";
		m_subDir = "";

		m_iTake = 0;
		m_tDelay = 0;

		m_quality = 100;
		m_bFlipRGB = false;
		m_bFlipD = false;
	}

	_PhotoTake::~_PhotoTake()
	{
	}

	bool _PhotoTake::init(void *pKiss)
	{
		IF_F(!this->_DataBase::init(pKiss));
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

		return true;
	}

	bool _PhotoTake::link(void)
	{
		IF_F(!this->_DataBase::link());
		IF_F(!m_pT->link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;

		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));

		return true;
	}

	bool _PhotoTake::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _PhotoTake::check(void)
	{
		NULL__(m_pV, -1);

		return this->_DataBase::check();
	}

	void _PhotoTake::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			m_pT->autoFPSto();
		}
	}

	void _PhotoTake::updateTake(void)
	{
	}

	bool _PhotoTake::startAutoMode(int nTake, int tInterval)
	{
		IF_F(check() < 0);
	}
	void _PhotoTake::stopAutoMode(void)
	{
	}

	bool _PhotoTake::shutter(void)
	{
		IF_F(check() < 0);

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

		// rgb
		Frame fBGR = *m_pV->BGR();
		if (m_bFlipRGB)
			fBGR = fBGR.flip(-1);
		Mat mBGR;
		fBGR.m()->copyTo(mBGR);
		IF_F(mBGR.empty());

		fName = m_subDir + i2str(m_iTake) + "_rgb.jpg";
		cv::imwrite(fName, mBGR, m_compress);
		cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
		system(cmd.c_str());

		LOG_I("RGB: " + fName);

		LOG_I("Take: " + i2str(m_iTake));
		m_iTake++;






                fName = getBaseDirSave();
                if (fName.empty())
                {
                    return false;
                }

                fName += tFormat();
                string imgName = fName + ".png";


		return true;
	}

	void _PhotoTake::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_DataBase::console(pConsole);
		IF_(check() < 0);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("iTake = " + i2str(m_iTake));
		pC->addMsg("Dir = " + m_subDir);
	}

}
