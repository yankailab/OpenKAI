/*
 * _ANR.cpp
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#include "_ANR.h"

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
	m_timeOut = USEC_1SEC;
	m_nCNdigit = 6;
	m_offsetRdigit = 0.01;
	m_wRdigit = 0.05;

	m_lp = "";
	m_lpBB.init();
	m_tStampLP = 0;

#ifdef USE_OCR
	m_bOCR = false;
	m_pOCR = NULL;
#endif

}

_ANR::~_ANR()
{
}

bool _ANR::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,cnPrefix);
	KISSm(pK,cnPosMargin);
	KISSm(pK,nCNdigit);
	KISSm(pK,offsetRdigit);
	KISSm(pK,wRdigit);

#ifdef USE_OCR
	KISSm(pK,bOCR);
#endif

	if(pK->v("timeOut",&m_timeOut))
		m_timeOut *= USEC_1SEC;

	string pPrefixList[N_PREFIX_CANDIDATE];
	int nP = pK->array("prefixList", pPrefixList, N_PREFIX_CANDIDATE);
	for(int i=0; i<nP; i++)
		m_vPrefixCandidate.push_back(pPrefixList[i]);

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_DetectorBaseCN", &iName));
	m_pDcn = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDcn, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_DetectorBaseLP", &iName));
	m_pDlp = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDlp, iName + " not found");

#ifdef USE_OCR
	IF_T(!m_bOCR);
	iName = "";
	F_INFO(pK->v("OCR", &iName));
	m_pOCR = (OCR*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pOCR, iName + " not found");
#endif

	return true;
}

bool _ANR::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _ANR::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		cn();
		lp();

		this->autoFPSto();
	}
}

int _ANR::check(void)
{
	NULL__(m_pDcn,-1);

	return 0;
}

string _ANR::char2Number(const char* pStr)
{
	string asc = "";
	if(!pStr)return asc;

	int i=0;
	while(pStr[i]!=0)
	{
		char c = pStr[i++];

		if(c=='O' ||
		   c=='o' ||
		   c=='D' ||
		   c=='Q' ||
		   c=='C')
			c='0';

		if(c=='I' ||
		   c=='L' ||
		   c=='J')
			c='1';

		if(c=='Z')
			c='2';

		asc += c;
	}

	return asc;
}

void _ANR::cn(void)
{
	IF_(check()<0);

	OBJECT* pO;
	int i = 0;
	while ((pO = m_pDcn->at(i++)) != NULL)
	{
		vFloat4 bb = pO->m_bb;
		string s = string(pO->m_pText);
		s = deleteNonASCII(s.c_str());

		bool bMatch = false;
		for(int j=0; j<m_vPrefixCandidate.size(); j++)
		{
			std::string::size_type pos = s.find(m_vPrefixCandidate[j]);
			IF_CONT(pos == std::string::npos);

			bMatch = true;
			break;
		}

		if(bMatch)
		{
			m_cnPrefixBB = bb;
			m_tStampCNprefix = getTimeUsec();
			break;
		}
	}

	if(getTimeUsec() - m_tStampCNprefix > m_timeOut)
	{
		m_cn = "";
		return;
	}

	i=0;
	while ((pO = m_pDcn->at(i++)) != NULL)
	{
		vFloat4 bb = pO->m_bb;

		IF_CONT(bb.x < m_cnPrefixBB.x);
		IF_CONT(abs(bb.y - m_cnPrefixBB.y) > m_cnPosMargin);
		IF_CONT(abs(bb.w - m_cnPrefixBB.w) > m_cnPosMargin);

		string s = string(pO->m_pText);
		s = char2Number(s.c_str());
		s = deleteNonNumber(s.c_str());
		IF_CONT(s.length() < m_nCNdigit);

		m_cn = m_cnPrefix + s.substr(0,m_nCNdigit);
		m_cnBB = bb;
		m_tStampCN = getTimeUsec();
	}

	if(getTimeUsec() - m_tStampCN > m_timeOut)
	{
		m_cn = "";
		return;
	}

	LOG_I("C Number: " + m_cn);
}

void _ANR::lp(void)
{
	IF_(check()<0);

#ifdef USE_OCR
	if(m_bOCR && m_pOCR)
	{
		vInt2 cs;
		cs.x = m_pDlp->m_fBGR.m()->cols;
		cs.y = m_pDlp->m_fBGR.m()->rows;

		m_lpBB.x = m_cnPrefixBB.x - m_cnPrefixBB.width() * 1.0;
		m_lpBB.y = m_cnPrefixBB.y + m_cnPrefixBB.height() * 18;
		m_lpBB.z = m_lpBB.x + m_cnPrefixBB.width() * 2.0;
		m_lpBB.w = m_lpBB.y + m_cnPrefixBB.height() * 3.5;

		m_lpBB.constrain(0.0,1.0);

		Rect r;
		r.x = m_lpBB.x * cs.x;
		r.width = (m_lpBB.z - m_lpBB.x) * cs.x;
		r.y = m_lpBB.y * cs.y;
		r.height = (m_lpBB.w - m_lpBB.y) * cs.y;

		m_pOCR->setFrame(m_pDlp->m_fBGR);
		m_lp = m_pOCR->scan(r);
	}
	else
	{
		OBJECT* pO;
		int i = 0;
		while ((pO = m_pDlp->at(i++)) != NULL)
		{
			string s = string(pO->m_pText);
//			IF_CONT(s.length() < 1);
//			IF_CONT(s.length() > 4);
			s = char2Number(s.c_str());
			s = deleteNonNumber(s.c_str());

			m_lp = s.substr(0,4);
			m_lpBB = pO->m_bb;

			if(m_bOCR && m_pOCR)
			{
				vInt2 cs;
				cs.x = m_pDlp->m_fBGR.m()->cols;
				cs.y = m_pDlp->m_fBGR.m()->rows;
				Rect r = pO->getRect(cs);

				m_pOCR->setFrame(m_pDlp->m_fBGR);
				m_lp = m_pOCR->scan(r);
			}

			m_tStampLP = getTimeUsec();
		}
	}
#endif

	LOG_I("L Number: " + m_lp);

	if(getTimeUsec() - m_tStampLP > m_timeOut)
	{
		m_lp = "";
		return;
	}
}

bool _ANR::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	IF_T(getTimeUsec() - m_tStampCN > m_timeOut);

	Scalar col = Scalar(0,0,255);

	putText(*pMat, string(m_cn),
			Point(10, pMat->rows*0.8),
			FONT_HERSHEY_SIMPLEX, 3.0, col, 5);

	Rect r;
	r.x = m_cnPrefixBB.x * pMat->cols;
	r.y = m_cnPrefixBB.y * pMat->rows;
	r.width = m_cnBB.z * pMat->cols - r.x;
	r.height = m_cnBB.w * pMat->rows - r.y;
	rectangle(*pMat, r, col, 2);

	putText(*pMat, string(m_lp),
			Point(10, pMat->rows*0.9),
			FONT_HERSHEY_SIMPLEX, 3.0, col, 5);

	r.x = m_lpBB.x * pMat->cols;
	r.y = m_lpBB.y * pMat->rows;
	r.width = m_lpBB.z * pMat->cols - r.x;
	r.height = m_lpBB.w * pMat->rows - r.y;
	rectangle(*pMat, r, col, 2);

	return true;
}

bool _ANR::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	if(getTimeUsec() - m_tStampCN > m_timeOut)
	{
		C_MSG("CN unrecognized");
	}
	else
	{
		C_MSG("CN: " + m_cn);
	}

	return true;
}

}
