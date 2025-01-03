#include "_APmavlink_video.h"

namespace kai
{

	_APmavlink_video::_APmavlink_video()
	{
		m_pAP = nullptr;
		m_pCurl = nullptr;
		m_fName = "";

		m_process = "";
		m_pFvid = nullptr;
		m_tRecStart = 0;

		m_bMeta = false;
		m_pFmeta = new _File();
		m_fCalib = "";
		m_dir = "";
	}

	_APmavlink_video::~_APmavlink_video()
	{
	}

	int _APmavlink_video::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("process", &m_process);
		pK->v("fName", &m_fName);
		pK->v("dir", &m_dir);
		pK->v("bMeta", &m_bMeta);

		return OK_OK;
	}

	int _APmavlink_video::link(void)
	{
		CHECK_(this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_APmavlink_base", &n);
		m_pAP = (_APmavlink_base *)(pK->findModule(n));
		NULL__(m_pAP, OK_ERR_NOT_FOUND);

		n = "";
		pK->v("_Uploader", &n);
		m_pCurl = (_Uploader *)(pK->findModule(n));

		pK->v("fCalib", &m_fCalib);
		readCamMatrices(m_fCalib, &m_mC, &m_mD);

		return OK_OK;
	}

	int _APmavlink_video::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_video::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _APmavlink_video::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();
			ON_RESUME;

			if (m_pFvid)
			{
				writeStream();
			}

			ON_PAUSE;
		}
	}

	void _APmavlink_video::onResume(void)
	{
		openStream();
	}

	void _APmavlink_video::onPause(void)
	{
		this->_ModuleBase::onPause();

		closeStream();
	}

	bool _APmavlink_video::openStream(void)
	{
		IF_F(check() != OK_OK);
		IF_F(m_pFvid);

		string strT = tFormat();
		m_fName = m_dir + strT;

		// open video stream
		string p = replace(m_process, "[fName]", m_fName + ".mka_t");
		m_pFvid = popen(p.c_str(), "r");
		if (!m_pFvid)
		{
			LOG_E("Failed to run command: " + p);
			return false;
		}

		m_tRecStart = getTbootMs();

		// open meta file
		IF__(!m_bMeta, true);
		IF__(!m_pFmeta->open(m_fName + ".json_t"), true);
		IF__(m_mC.empty(), true);
		IF__(m_mD.empty(), true);

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

		m += "\x0d\x0a";
		m_pFmeta->write((uint8_t *)m.c_str(), m.length());

		return true;
	}

	void _APmavlink_video::closeStream(void)
	{
		IF_(!m_pFvid);

		pclose(m_pFvid);
		m_pFvid = NULL;
		m_tRecStart = 0;

		string cmd;
		cmd = "mv " + m_fName + ".mka_t " + m_fName + ".mka";
		system(cmd.c_str());
		if (m_pCurl)
			m_pCurl->addFile(m_fName + ".mka");

		IF_(!m_bMeta);
		IF_(!m_pFmeta->bOpen());
		m_pFmeta->close();
		cmd = "mv " + m_fName + ".json_t " + m_fName + ".json";
		system(cmd.c_str());
		if (m_pCurl)
			m_pCurl->addFile(m_fName + ".json");
	}

	void _APmavlink_video::writeStream(void)
	{
		IF_(!m_bMeta);
		IF_(!m_pFvid);
		IF_(!m_pFmeta->bOpen());

		uint64_t tFrame = getTbootMs() - m_tRecStart;
		vDouble4 vP = m_pAP->getGlobalPos();
		vFloat3 vA = m_pAP->getAttitude();

		object jo;
		JO(jo, "tFrame", (double)tFrame);
		JO(jo, "lat", lf2str(vP.x, 8));
		JO(jo, "lon", lf2str(vP.y, 8));
		JO(jo, "alt", lf2str(vP.z, 5));
		JO(jo, "rAlt", lf2str(vP.w, 5));
		JO(jo, "hdg", lf2str(m_pAP->getHdg(), 5));
		JO(jo, "yaw", lf2str(vA.x, 5));
		JO(jo, "pitch", lf2str(vA.y, 5));
		JO(jo, "roll", lf2str(vA.z, 5));
		string m = picojson::value(jo).serialize();

		m += "\x0d\x0a";
		m_pFmeta->write((uint8_t *)m.c_str(), m.length());
	}

	void _APmavlink_video::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(check() != OK_OK);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("fName = " + m_fName);
	}

}
