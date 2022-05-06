#include "_AP_video.h"

namespace kai
{

	_AP_video::_AP_video()
	{
		m_pAP = NULL;
		m_pV = NULL;

		m_vSize.set(1280, 720);
		m_dir = "/home/";
		m_saveDir = "";
		m_bRecording = false;
	}

	_AP_video::~_AP_video()
	{
	}

	bool _AP_video::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("dir", &m_dir);
		pK->v("gstOutput", &m_gstOutput);

		string n;

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->getInst(n));
		NULL_Fl(m_pAP, n + ": not found");

		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));

		Kiss* pKv = pK->find(n);
		if(pKv)
			pKv->v("vSize", &m_vSize);

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
		openStream();

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();
			this->_StateBase::update();

			// if (bStateChanged())
			// {
			// 	if (this->bActive())
			// 		openStream();
			// 	else
			// 		closeStream();
			// }

			if (m_bRecording)
			{
				writeStream();
			}

			m_pT->autoFPSto();
		}
	}

	bool _AP_video::openStream(void)
	{
		IF_F(check() < 0);
		IF_F(m_bRecording);

		string strT = tFormat();
		m_saveDir = m_dir + strT + "/";

		string cmd;
		// cmd = "mkdir /media/usb";
		// system(cmd.c_str());
		// cmd = "mount /dev/sda1 /media/usb";
		// system(cmd.c_str());
		cmd = "mkdir " + m_saveDir;
		system(cmd.c_str());

		string gst = replace(m_gstOutput, "[fName]", m_saveDir + strT);
		if (!m_gst.open(gst,
						CAP_GSTREAMER,
						0,
						30,
						cv::Size(m_vSize.x, m_vSize.y),
						true))
		{
			LOG_E("Cannot open GStreamer output:" + gst);
			return false;
		}

		m_bRecording = true;
		return true;
	}

	void _AP_video::closeStream(void)
	{
		IF_(!m_bRecording);

		m_bRecording = false;
	}

	void _AP_video::writeStream(void)
	{
		IF_(!m_gst.isOpened());

		vDouble4 vP;
		vP.init();
		vP = m_pAP->getGlobalPos();
		string lat = lf2str(vP.x, 7);
		string lon = lf2str(vP.y, 7);
		string alt = lf2str(vP.z, 3);

		Frame fBGR = *m_pV->BGR();
		IF_(fBGR.bEmpty());

		m_gst << *fBGR.m();
	}

	void _AP_video::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_StateBase::console(pConsole);
		IF_(check() < 0);
		msgActive(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("Dir = " + m_saveDir);
	}

}
