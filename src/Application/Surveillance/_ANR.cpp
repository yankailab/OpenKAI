/*
 * _ANR.cpp
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#include "_ANR.h"

#ifdef USE_OPENCV

namespace kai
{

	_ANR::_ANR()
	{
		m_pDcn = NULL;
		m_pDlp = NULL;
		m_cnPrefix = "";
		m_cnPosMargin = 0.05;
		m_cn = "";
		m_cnPrefixBB.init();
		m_cnBB.init();
		m_tStampCNprefix = 0;
		m_tStampCN = 0;
		m_timeOut = SEC_2_USEC;
		m_nCNdigit = 6;
		m_offsetRdigit = 0.01;
		m_wRdigit = 0.05;

		m_lp = "";
		m_lpBB.init();
		m_tStampLP = 0;

		m_tempFile = "anr.jpg";
		m_shFile = "alpr.sh";
		m_vAlprROI.init();
		m_vAlprROI.z = 1.0;
		m_vAlprROI.w = 1.0;
		m_pWS = NULL;

#ifdef USE_OCR
		m_bOCR = false;
		m_pOCR = NULL;
#endif
	}

	_ANR::~_ANR()
	{
	}

	bool _ANR::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("cnPrefix", &m_cnPrefix);
		pK->v("cnPosMargin", &m_cnPosMargin);
		pK->v("nCNdigit", &m_nCNdigit);
		pK->v("offsetRdigit", &m_offsetRdigit);
		pK->v("wRdigit", &m_wRdigit);

		Kiss *pA = pK->child("oalpr");
		if (pA)
		{
			int jpgQuality = 80;
			pA->v("jpgQuality", &jpgQuality);
			m_vJPGquality.push_back(IMWRITE_JPEG_QUALITY);
			m_vJPGquality.push_back(jpgQuality);
			pA->v("x", &m_vAlprROI.x);
			pA->v("y", &m_vAlprROI.y);
			pA->v("z", &m_vAlprROI.z);
			pA->v("w", &m_vAlprROI.w);
			pA->v("tempFile", &m_tempFile);
			pA->v("shFile", &m_shFile);
		}

#ifdef USE_OCR
		KISSm(pK, bOCR);
#endif

		if (pK->v("timeOut", &m_timeOut))
			m_timeOut *= SEC_2_USEC;

		vector<string> vPrefix;
		pK->a("vPrefix", &vPrefix);
		for (int i = 0; i < vPrefix.size(); i++)
			m_vPrefixCandidate.push_back(vPrefix[i]);

		string n;

		n = "";
		F_ERROR_F(pK->v("_DetectorBaseCN", &n));
		m_pDcn = (_DetectorBase *)(pK->getInst(n));
		IF_Fl(!m_pDcn, n + " not found");

		n = "";
		F_ERROR_F(pK->v("_DetectorBaseLP", &n));
		m_pDlp = (_DetectorBase *)(pK->getInst(n));
		IF_Fl(!m_pDlp, n + " not found");

		n = "";
		F_INFO(pK->v("_WebSocket", &n));
		m_pWS = (_WebSocket *)(pK->getInst(n));
		IF_Fl(!m_pWS, n + " not found");

#ifdef USE_OCR
		IF_T(!m_bOCR);
		n = "";
		F_INFO(pK->v("OCR", &n));
		m_pOCR = (OCR *)(pK->getInst(n));
		IF_Fl(!m_pOCR, n + " not found");
#endif

		return true;
	}

	bool _ANR::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _ANR::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if (cn())
			{
				lpO();
			}

			//		lp();

			m_pT->autoFPSto();
		}
	}

	int _ANR::check(void)
	{
		NULL__(m_pDcn, -1);
		NULL__(m_pDlp, -1);
		NULL__(m_pWS, -1);

		return 0;
	}

	string _ANR::char2Number(const char *pStr)
	{
		string asc = "";
		if (!pStr)
			return asc;

		int i = 0;
		while (pStr[i] != 0)
		{
			char c = pStr[i++];

			if (c == 'O' ||
				c == 'o' ||
				c == 'D' ||
				c == 'Q' ||
				c == 'C')
				c = '0';

			if (c == 'I' ||
				c == 'L' ||
				c == 'J')
				c = '1';

			if (c == 'Z')
				c = '2';

			asc += c;
		}

		return asc;
	}

	bool _ANR::cn(void)
	{
		IF__(check() < 0, false);

		_Object *pO;
		int i = 0;
		while ((pO = m_pDcn->m_pU->get(i++)) != NULL)
		{
			string s = string(pO->getText());
			s = deleteNonASCII(s.c_str());

			bool bMatch = false;
			for (int j = 0; j < m_vPrefixCandidate.size(); j++)
			{
				std::string::size_type pos = s.find(m_vPrefixCandidate[j]);
				IF_CONT(pos == std::string::npos);

				bMatch = true;
				break;
			}

			if (bMatch)
			{
				m_cnPrefixBB = pO->getBB2D();
				m_tStampCNprefix = getApproxTbootUs();
				break;
			}
		}

		if (getApproxTbootUs() - m_tStampCNprefix > m_timeOut)
		{
			m_cn = "";
			return false;
		}

		string cn;
		i = 0;
		while ((pO = m_pDcn->m_pU->get(i++)) != NULL)
		{
			vFloat4 bb = pO->getBB2D();

			IF_CONT(bb.x < m_cnPrefixBB.x);
			IF_CONT(abs(bb.y - m_cnPrefixBB.y) > m_cnPosMargin);
			IF_CONT(abs(bb.w - m_cnPrefixBB.w) > m_cnPosMargin);

			string s = string(pO->getText());
			s = char2Number(s.c_str());
			s = deleteNonNumber(s.c_str());
			IF_CONT(s.length() < m_nCNdigit);

			cn = m_cnPrefix + s.substr(0, m_nCNdigit);
			m_cnBB = bb;
			m_tStampCN = getApproxTbootUs();
		}

		if (getApproxTbootUs() - m_tStampCN > m_timeOut)
		{
			m_cn = "";
			return false;
		}

		IF__(cn == m_cn, false);

		m_cn = cn;
		return true;
	}

	void _ANR::lpO(void)
	{
		IF_(check() < 0);
		IF_(m_cn.empty());

		NULL_(m_pDlp->m_pV);
		IF_(m_pDlp->m_pV->BGR()->bEmpty());
		m_fBGR.copy(*m_pDlp->m_pV->BGR());
		cv::imwrite(m_tempFile, *m_fBGR.m(), m_vJPGquality);

		FILE *fp;
		char path[1035];

		fp = popen(m_shFile.c_str(), "r");
		NULL_l(fp, "Failed to run command:" + m_shFile);

		while (fgets(path, sizeof(path) - 1, fp))
			;
		pclose(fp);

		string strR = string(path);
		std::string::size_type k;

		k = strR.find("\\u");
		while (k != std::string::npos)
		{
			strR.replace(k, 6, "-");
			k = strR.find("\\u");
		}

		k = strR.find("\"");
		while (k != std::string::npos)
		{
			strR.erase(k, 1);
			k = strR.find("\"");
		}

		k = strR.find(":");
		while (k != std::string::npos)
		{
			strR.erase(k, 1);
			k = strR.find(":");
		}

		k = strR.find(" plate");
		if (k == std::string::npos)
			return;

		strR.erase(k, 7);
		std::string::size_type m;
		m = strR.find(',', k);

		string cStr = strR.substr(k, m - k);
		cStr += " " + m_cn;

		m_pWS->write((unsigned char *)cStr.c_str(),
					 cStr.length(),
					 WS_MODE_TXT);

		if (getApproxTbootUs() - m_tStampLP > m_timeOut)
		{
			m_lp = "";
			return;
		}
	}

	void _ANR::lp(void)
	{
		IF_(check() < 0);

#ifdef USE_OCR
		if (m_bOCR && m_pOCR)
		{
			vInt2 cs;
			cs.x = m_pDlp->m_fBGR.m()->cols;
			cs.y = m_pDlp->m_fBGR.m()->rows;

			m_lpBB.x = m_cnPrefixBB.x - m_cnPrefixBB.width() * 1.0;
			m_lpBB.y = m_cnPrefixBB.y + m_cnPrefixBB.height() * 18;
			m_lpBB.z = m_lpBB.x + m_cnPrefixBB.width() * 2.0;
			m_lpBB.w = m_lpBB.y + m_cnPrefixBB.height() * 3.5;

			m_lpBB.constrain(0.0, 1.0);

			Rect r;
			r.x = m_lpBB.x * cs.x;
			r.width = (m_lpBB.z - m_lpBB.x) * cs.x;
			r.y = m_lpBB.y * cs.y;
			r.height = (m_lpBB.w - m_lpBB.y) * cs.y;

			m_pOCR->setFrame(m_pDlp->m_fBGR);
			m_lp = m_pOCR->scan(&r);
		}
		else
		{
			OBJECT *pO;
			int i = 0;
			while ((pO = m_pDlp->at(i++)) != NULL)
			{
				string s = string(pO->m_pText);
				//			IF_CONT(s.length() < 1);
				//			IF_CONT(s.length() > 4);
				s = char2Number(s.c_str());
				s = deleteNonNumber(s.c_str());

				m_lp = s.substr(0, 4);
				m_lpBB = pO->m_bb;

				if (m_bOCR && m_pOCR)
				{
					vInt2 cs;
					cs.x = m_pDlp->m_fBGR.m()->cols;
					cs.y = m_pDlp->m_fBGR.m()->rows;
					Rect r = convertBB<vInt4>(convertBB(pO->m_bb, cs));

					m_pOCR->setFrame(m_pDlp->m_fBGR);
					m_lp = m_pOCR->scan(&r);
				}

				m_tStampLP = getApproxTbootUs();
			}
		}
#endif

		LOG_I("L Number: " + m_lp);

		if (getApproxTbootUs() - m_tStampLP > m_timeOut)
		{
			m_lp = "";
			return;
		}
	}

	void _ANR::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		if (getApproxTbootUs() - m_tStampCN > m_timeOut)
			pC->addMsg("CN unrecognized");
		else
			pC->addMsg("CN: " + m_cn);
	}

	void _ANR::cvDraw(void *pWindow)
	{
		NULL_(pWindow);
		this->_ModuleBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Frame *pF = pWin->getNextFrame();
		NULL_(pF);
		Mat *pM = pF->m();
		IF_(pM->empty());

		IF_(getApproxTbootUs() - m_tStampCN > m_timeOut);

		Scalar col = Scalar(0, 0, 255);

		putText(*pM, string(m_cn),
				Point(10, pM->rows * 0.8),
				FONT_HERSHEY_SIMPLEX, 3.0, col, 5);

		Rect r;
		r.x = m_cnPrefixBB.x * pM->cols;
		r.y = m_cnPrefixBB.y * pM->rows;
		r.width = m_cnBB.z * pM->cols - r.x;
		r.height = m_cnBB.w * pM->rows - r.y;
		rectangle(*pM, r, col, 2);

		putText(*pM, string(m_lp),
				Point(10, pM->rows * 0.9),
				FONT_HERSHEY_SIMPLEX, 3.0, col, 5);

		r.x = m_lpBB.x * pM->cols;
		r.y = m_lpBB.y * pM->rows;
		r.width = m_lpBB.z * pM->cols - r.x;
		r.height = m_lpBB.w * pM->rows - r.y;
		rectangle(*pM, r, col, 2);
	}

}
#endif
