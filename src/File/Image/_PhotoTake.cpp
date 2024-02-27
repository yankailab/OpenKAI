#include "_PhotoTake.h"

namespace kai
{

	_PhotoTake::_PhotoTake()
	{
		m_pV = NULL;
		m_vPos.clear();
		m_bfProcess.clearAll();

		m_iTake = 0;
		m_nTake = 0;
		m_tDelay = 0;
		m_bAuto = false;
		m_ieShutter.init(USEC_1SEC);

		m_dir = "/home/";
		m_subDir = "";
		m_bFlip = false;
		m_quality = 100;
	}

	_PhotoTake::~_PhotoTake()
	{
	}

	bool _PhotoTake::init(void *pKiss)
	{
		IF_F(!this->_FileBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
    	

		pK->v("quality", &m_quality);
		pK->v("bFlip", &m_bFlip);
		pK->v("tDelay", &m_tDelay);
		pK->v("bAuto", &m_bAuto);
		pK->v("dir", &m_dir);
		pK->v("subDir", &m_subDir);
		pK->v("nTake", &m_nTake);

		int tInt = USEC_1SEC;
		pK->v("tInterval", &tInt);
		m_ieShutter.init(tInt);

		if (m_subDir.empty())
			m_subDir = m_dir + tFormat() + "/";
		else
			m_subDir = m_dir + m_subDir;

		int bClearDir;
		pK->v("bClearDir", &bClearDir);
		if (bClearDir)
			removeDir(m_subDir);

		createDir(m_subDir);

		m_compress.push_back(IMWRITE_JPEG_QUALITY);
		m_compress.push_back(m_quality);

		return true;
	}

	bool _PhotoTake::link(void)
	{
		IF_F(!this->_FileBase::link());
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

		return this->_FileBase::check();
	}

	void _PhotoTake::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			updateTake();

			m_pT->autoFPSto();
		}
	}

	void _PhotoTake::updateTake(void)
	{
		IF_(check() < 0);

		if (m_bfProcess.b(pt_shutter, true))
			take();

		IF_(!m_bAuto);

		if (m_ieShutter.update(getApproxTbootUs()))
			take();
	}

	void _PhotoTake::setPos(const vDouble3 &vPos)
	{
		m_vPos = vPos;
	}

	void _PhotoTake::setDelay(const uint64_t &tDelay)
	{
		m_tDelay = tDelay;
	}

	bool _PhotoTake::shutter(void)
	{
		m_bfProcess.set(pt_shutter);
		return true;
	}

	void _PhotoTake::setInterval(const uint64_t &tInterval)
	{
		m_ieShutter.init(tInterval);
	}

	bool _PhotoTake::startAutoMode(int nTake)
	{
		m_nTake = nTake;
		m_bAuto = true;
		return true;
	}

	void _PhotoTake::stopAutoMode(void)
	{
		m_bAuto = false;
	}

	bool _PhotoTake::take(void)
	{
		IF_F(check() < 0);

		if (m_tDelay > 0)
			m_pT->sleepT(m_tDelay);

		if(m_nTake > 0)
		{
			if(m_iTake >= m_nTake)
			{
				m_iTake = 0;
				m_bAuto = false;
				return false;
			}
		}

		string lat = lf2str(m_vPos.x, 7);
		string lon = lf2str(m_vPos.y, 7);
		string alt = lf2str(m_vPos.z, 3);

		// rgb
		Frame fBGR = *m_pV->getFrameRGB();
		IF_F(fBGR.bEmpty());
		if (m_bFlip)
			fBGR = fBGR.flip(-1);
		Mat mBGR;
		fBGR.m()->copyTo(mBGR);

		string fName = m_subDir + tFormat() + ".jpg";
		cv::imwrite(fName, mBGR, m_compress);

		if (m_vPos.x != 0)
		{
			string cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
			system(cmd.c_str());
		}

		LOG_I("Take: " + fName);
		m_iTake++;

		return true;
	}

	void _PhotoTake::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_FileBase::console(pConsole);
		IF_(check() < 0);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("iTake = " + i2str(m_iTake));
		pC->addMsg("Dir = " + m_subDir);
	}

}
