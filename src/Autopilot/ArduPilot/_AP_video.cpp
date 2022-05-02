#include "_AP_video.h"

namespace kai
{

	_AP_video::_AP_video()
	{
		m_pAP = NULL;
		m_pV = NULL;

		m_vSize.set(1280,720);
		m_dir = "/home/";
		m_subDir = "";
	}

	_AP_video::~_AP_video()
	{
	}

	bool _AP_video::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vSize", &m_vSize);
		pK->v("dir", &m_dir);

		if (m_subDir.empty())
			m_subDir = m_dir + tFormat() + "/";
		else
			m_subDir = m_dir + m_subDir;

		string n;

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->getInst(n));
		NULL_Fl(m_pAP, n + ": not found");

		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));

		return true;
	}

	bool _AP_video::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_video::check(void)
	{
		NULL__(m_pAP, -1);
		NULL__(m_pV, -1);

		return this->_StateBase::check();
	}

	void _AP_video::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();
			this->_StateBase::update();

			if (bStateChanged())
			{
				if (this->bActive())
				{
					openStream();
				}
				else
				{
					closeStream();
				}
			}

			if (bActive())
			{
				writeStream();
			}

			m_pT->autoFPSto();
		}
	}

	bool _AP_video::openStream(void)
	{
		IF_F(check() < 0);

		string cmd;
		cmd = "mkdir /media/usb";
		system(cmd.c_str());
		cmd = "mount /dev/sda1 /media/usb";
		system(cmd.c_str());
		cmd = "mkdir " + m_subDir;
		system(cmd.c_str());

		if (!m_gst.open(m_gstOutput,
						CAP_GSTREAMER,
						0,
						30,
						cv::Size(m_vSize.x, m_vSize.y),
						true))
		{
			LOG_E("Cannot open GStreamer output");
			return false;
		}

		return true;
	}

	void _AP_video::closeStream(void)
	{
	}

	void _AP_video::writeStream(void)
	{
		vDouble4 vP;
		vP.init();
		vP = m_pAP->getGlobalPos();
		string lat = lf2str(vP.x, 7);
		string lon = lf2str(vP.y, 7);
		string alt = lf2str(vP.z, 3);

		Frame fBGR = *m_pV->BGR();
		if (m_bFlipRGB)
			fBGR = fBGR.flip(-1);
		Mat mBGR;
		fBGR.m()->copyTo(mBGR);
		IF_(mBGR.empty());

		Frame F, F2;
		if (m_gst.isOpened())
		{
			F.copy(fBGR);
			Size fs = F.size();

			if (fs.width != m_vSize.x || fs.height != m_vSize.y)
			{
				F2 = F.resize(m_vSize.x, m_vSize.y);
				F = F2;
			}

			m_gst << *F.m();
		}
	}

	void _AP_video::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_StateBase::console(pConsole);
		IF_(check() < 0);
		msgActive(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("Dir = " + m_subDir);
	}

}
