#include "_AP_videoStream.h"

namespace kai
{

	_AP_videoStream::_AP_videoStream()
	{
		m_pAP = NULL;
		m_pCurl = NULL;
		m_fName = "";
		m_process = "";
		m_pFvid = NULL;
		m_dir = "";

		m_iWP = INT_MAX;
	}

	_AP_videoStream::~_AP_videoStream()
	{
	}

	bool _AP_videoStream::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("process", &m_process);
		pK->v("fName", &m_fName);
		pK->v("dir", &m_dir);
		pK->a("vWP", &m_vWP);

		return true;
	}

	bool _AP_videoStream::link(void)
	{
		IF_F(!this->_StateBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->getInst(n));
		NULL_Fl(m_pAP, n + ": not found");

		n = "";
		pK->v("_Curl", &n);
		m_pCurl = (_Curl *)(pK->getInst(n));

		return true;
	}

	bool _AP_videoStream::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_videoStream::check(void)
	{
		NULL__(m_pAP, -1);

		return this->_StateBase::check();
	}

	void _AP_videoStream::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();
			this->_StateBase::update();

			updateStream();

			m_pT->autoFPSto();
		}
	}

	void _AP_videoStream::updateStream(void)
	{
		IF_(check() < 0);

		int iSeq = m_pAP->getWPseq();
		IF_(iSeq == m_iWP);
		if (!count(m_vWP.begin(), m_vWP.end(), iSeq))
		{
			closeStream();
			m_iWP = INT_MAX;
			return;
		}

		if(openStream())
		{
			m_iWP = iSeq;
		}
	}

	bool _AP_videoStream::openStream(void)
	{
		IF_F(check() < 0);
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

		return true;
	}

	void _AP_videoStream::closeStream(void)
	{
		IF_(!m_pFvid);

		pclose(m_pFvid);
		m_pFvid = NULL;

		string cmd;
		cmd = "mv " + m_fName + ".mka_t " + m_fName + ".mka";
		system(cmd.c_str());
		if (m_pCurl)
			m_pCurl->addFile(m_fName + ".mka");
	}

	void _AP_videoStream::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_StateBase::console(pConsole);
		IF_(check() < 0);
		msgActive(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("fName = " + m_fName);
	}

}
