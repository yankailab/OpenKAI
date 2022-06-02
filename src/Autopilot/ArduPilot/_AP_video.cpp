#include "_AP_video.h"

namespace kai
{

	_AP_video::_AP_video()
	{
		m_pAP = NULL;
		m_pV = NULL;
		m_pCurl = NULL;
		m_pF = new _File();

		m_vSize.set(1280, 720);
		m_fCalib = "";
		m_dir = "/home/lab/";
		m_fName = "";
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
		pK->v("fName", &m_fName);

		string n;

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->getInst(n));
		NULL_Fl(m_pAP, n + ": not found");

		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));

		n = "";
		pK->v("_Curl", &n);
		m_pCurl = (_Curl *)(pK->getInst(n));

		Kiss* pKv = pK->find(n);
		if(pKv)
			pKv->v("vSize", &m_vSize);


		pK->v("fCalib", &m_fCalib);
		readCamMatrices(m_fCalib, &m_mC, &m_mD);

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
		m_fName = m_dir + strT;

		// open video stream
		string gst = replace(m_gstOutput, "[fName]", m_fName + ".mka");
		if (!m_gst.open(gst,
						CAP_GSTREAMER,
						0,
						m_pT->getTargetFPS(),
						cv::Size(m_vSize.x, m_vSize.y),
						true))
		{
			LOG_E("Cannot open GStreamer output:" + gst);
			return false;
		}

		// open meta file
		IF_F(!m_pF->open(m_fName + ".json"));
        object jo;
        JO(jo, "name", "calib");
        JO(jo, "Fx", m_mC.at<double>(0, 0));
        JO(jo, "Fy", m_mC.at<double>(1, 1));
        JO(jo, "Cx", m_mC.at<double>(0, 2));
        JO(jo, "Cy", m_mC.at<double>(1, 2));
        JO(jo, "k1", m_mD.at<double>(0, 0));
        JO(jo, "k2", m_mD.at<double>(0, 1));
        JO(jo, "p1", m_mD.at<double>(0, 2));
        JO(jo, "p2", m_mD.at<double>(0, 3));
        JO(jo, "k3", m_mD.at<double>(0, 4));
        string m = picojson::value(jo).serialize();
		m_pF->writeLine((uint8_t*)m.c_str(), m.length());

		m_iFrame = 0;
		m_tRecStart = getTbootMs();

		m_bRecording = true;
		return true;
	}

	void _AP_video::closeStream(void)
	{
		IF_(!m_bRecording);

		m_bRecording = false;
		m_iFrame = 0;
		m_tRecStart = 0;
		m_gst.release();
		m_pF->close();

		if(m_pCurl)
		{
			m_pCurl->addFile(m_fName + ".json");
			m_pCurl->addFile(m_fName + ".mka");
		}
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
		pC->addMsg("fName = " + m_fName);
		pC->addMsg("iFrame = " + i2str(m_iFrame));
	}

}
