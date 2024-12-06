#include "_APmavlink_videoStream.h"

namespace kai
{

	_APmavlink_videoStream::_APmavlink_videoStream()
	{
		m_pAP = NULL;
		m_pCurl = NULL;
		m_fName = "";
		m_process = "";
		m_dir = "";

		m_iWP = INT_MAX;

		m_gstPID = 0;
		m_tVidInt = 10;
		m_tRecStart = 0;
	}

	_APmavlink_videoStream::~_APmavlink_videoStream()
	{
	}

	int _APmavlink_videoStream::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("process", &m_process);
		pK->v("fName", &m_fName);
		pK->v("dir", &m_dir);
		pK->a("vWP", &m_vWP);
		pK->v("tVidInt", &m_tVidInt);

		return OK_OK;
	}

	int _APmavlink_videoStream::link(void)
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

		return OK_OK;
	}

	int _APmavlink_videoStream::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_videoStream::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _APmavlink_videoStream::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateStream();

		}
	}

	void _APmavlink_videoStream::updateStream(void)
	{
		IF_(check() != OK_OK);

		uint64_t tNow = getApproxTbootUs();

		int iSeq = m_pAP->getWPseq();
		if (iSeq == m_iWP)
		{
			uint64_t tRec = tNow - m_tRecStart;
			IF_(tRec < m_tVidInt * USEC_1SEC);
		}

		closeStream();

		if (!count(m_vWP.begin(), m_vWP.end(), iSeq))
		{
			m_iWP = INT_MAX;
			return;
		}

		if (openStream())
		{
			m_iWP = iSeq;
			m_tRecStart = tNow;
		}
	}

	bool _APmavlink_videoStream::openStream(void)
	{
		IF_F(check() != OK_OK);
		// IF_F(m_pFvid);
		IF_F(m_gstPID);

		string strT = tFormat();
		m_fName = m_dir + strT;

		// open video stream
		string p = replace(m_process, "[fName]", m_fName + ".mkv_t");
		int infp, outfp;
		m_gstPID = popen2(p.c_str(), &infp, &outfp);
		if (m_gstPID <= 0)
		{
			LOG_E("Failed to run command: " + p);
			return false;
		}

		return true;
	}

	void _APmavlink_videoStream::closeStream(void)
	{
		IF_(!m_gstPID);

		kill(m_gstPID+1, SIGKILL);
		m_gstPID = 0;

		string cmd;
		cmd = "mv " + m_fName + ".mkv_t " + m_fName + ".mkv";
		system(cmd.c_str());
		if (m_pCurl)
			m_pCurl->addFile(m_fName + ".mkv");
	}

	void _APmavlink_videoStream::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(check() != OK_OK);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("fName = " + m_fName);
	}

}
