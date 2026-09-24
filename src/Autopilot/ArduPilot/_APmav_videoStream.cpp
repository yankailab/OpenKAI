#include "_APmav_videoStream.h"

namespace kai
{

	_APmav_videoStream::_APmav_videoStream()
	{
	}

	_APmav_videoStream::~_APmav_videoStream()
	{
	}

	bool _APmav_videoStream::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		jKv(j, "process", m_process);
		jKv(j, "fName", m_fName);
		jKv(j, "dir", m_dir);
		jKv(j, "vWP", m_vWP);
		jKv(j, "tVidInt", m_tVidInt);

		return true;
	}

	bool _APmav_videoStream::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n;

		n = "";
		jKv(j, "_APmav_base", n);
		m_pAP = (_APmav_base *)(pM->findModule(n));
		NULL_F(m_pAP);

		n = "";
		jKv(j, "_Uploader", n);
		m_pCurl = (_Uploader *)(pM->findModule(n));

		return true;
	}

	bool _APmav_videoStream::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmav_videoStream::check(void)
	{
		NULL_F(m_pAP);

		return this->_ModuleBase::check();
	}

	void _APmav_videoStream::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateStream();
		}
	}

	void _APmav_videoStream::updateStream(void)
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

	bool _APmav_videoStream::openStream(void)
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

	void _APmav_videoStream::closeStream(void)
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

	void _APmav_videoStream::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(!check());

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("fName = " + m_fName);
	}

}
