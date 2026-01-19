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

	bool _APmavlink_videoStream::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		jVar(j, "process", m_process);
		jVar(j, "fName", m_fName);
		jVar(j, "dir", m_dir);
		jVar(j, "vWP", m_vWP);
		jVar(j, "tVidInt", m_tVidInt);

		return true;
	}

	bool _APmavlink_videoStream::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n;

		n = "";
		jVar(j, "_APmavlink_base", n);
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL_F(m_pAP);

		n = "";
		jVar(j, "_Uploader", n);
		m_pCurl = (_Uploader *)(pM->findModule(n));

		return true;
	}

	bool _APmavlink_videoStream::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _APmavlink_videoStream::check(void)
	{
		NULL_F(m_pAP);

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
		IF_(!check());

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
		IF_F(!check());
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

		kill(m_gstPID + 1, SIGKILL);
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
		IF_(!check());

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("fName = " + m_fName);
	}

}
