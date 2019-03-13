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
	m_pD = NULL;
	m_cnPrefix = "";
	m_cnPosMargin = 0.05;
	m_cn = "";
	m_cnPrefixBB.init();
	m_cnBB.init();
	m_tStampCNprefix = 0;
	m_tStampCN = 0;
	m_timeOut = USEC_1SEC;
	m_nCNdigit = 6;
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

	if(pK->v("timeOut",&m_timeOut))
		m_timeOut *= USEC_1SEC;

	string pPrefixList[N_PREFIX_CANDIDATE];
	int nP = pK->array("prefixList", pPrefixList, N_PREFIX_CANDIDATE);
	for(int i=0; i<nP; i++)
		m_vPrefixCandidate.push_back(pPrefixList[i]);

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pD = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pD, iName + " not found");

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
//		lp();

		this->autoFPSto();
	}
}

int _ANR::check(void)
{
	NULL__(m_pD,-1);

	return 0;
}

void _ANR::cn(void)
{
	IF_(check()<0);

	OBJECT* pO;
	int i = 0;
	while ((pO = m_pD->at(i++)) != NULL)
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
	while ((pO = m_pD->at(i++)) != NULL)
	{
		vFloat4 bb = pO->m_bb;

		IF_CONT(bb.x < m_cnPrefixBB.z);
		IF_CONT(abs(bb.y - m_cnPrefixBB.y) > m_cnPosMargin);
		IF_CONT(abs(bb.w - m_cnPrefixBB.w) > m_cnPosMargin);

		string s = string(pO->m_pText);
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

	LOG_I("Number: " + m_cn);
}

void _ANR::lp(void)
{
	IF_(check()<0);

	LOG_I("Number: ");
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
