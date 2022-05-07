#include "_AP_video.h"

namespace kai
{

	_AP_video::_AP_video()
	{
		m_pAP = NULL;
		m_pV = NULL;
		m_pF = new _File();

		m_vSize.set(1280, 720);
		m_dir = "/home/lab/";
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
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();
			this->_StateBase::update();

			if (bStateChanged())
			{
				if (this->bActive())
					openStream();
				else
					closeStream();
			}

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

		string fName = m_saveDir + strT;

		// open video stream
		string gst = replace(m_gstOutput, "[fName]", fName);
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

		// open meta file
		IF_F(!m_pF->open(fName + ".json"));

		m_iFrame = 0;
		m_tRecStart = getTbootMs();

		m_bRecording = true;
		return true;
	}

	void _AP_video::closeStream(void)
	{
		IF_(!m_bRecording);

		m_iFrame = 0;
		m_tRecStart = 0;
		m_pF->close();
		m_gst.release();

		m_bRecording = false;
	}

	void _AP_video::writeStream(void)
	{
		IF_(!m_gst.isOpened());

		// ouput one frame
		Frame fBGR = *m_pV->BGR();
		IF_(fBGR.bEmpty());
		m_gst << *fBGR.m();

		// output meta data
		m_iFrame++;
		uint64_t tFrame = getTbootMs() - m_tRecStart;
		vDouble4 vP = m_pAP->getGlobalPos();
		vFloat3 vA = m_pAP->getApAttitude();

        object jo;
        JO(jo, "iFrame", (double)m_iFrame);
        JO(jo, "tFrame", (double)tFrame);
        JO(jo, "lat", lf2str(vP.x, 7));
        JO(jo, "lon", lf2str(vP.y, 7));
        JO(jo, "alt", (double)vP.z);
        JO(jo, "yaw", (double)vA.x);
        JO(jo, "pitch", (double)vA.y);
        JO(jo, "roll", (double)vA.z);
        string m = picojson::value(jo).serialize();
		m_pF->writeLine((uint8_t*)m.c_str(), m.length());
	}

	void _AP_video::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_StateBase::console(pConsole);
		IF_(check() < 0);
		msgActive(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("saveDir = " + m_saveDir);
		pC->addMsg("iFrame = " + i2str(m_iFrame));
	}

}
