/*
 * _PCregistration.cpp
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#include "_PCregistration.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D

namespace kai
{

_PCregistration::_PCregistration()
{
	m_thr = 0.02;
	m_nMinP = 1000;
}

_PCregistration::~_PCregistration()
{
}

bool _PCregistration::init(void *pKiss)
{
	IF_F(!_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("thr", &m_thr);
	pK->v("nMinP", &m_nMinP);

	Kiss* pPair = pK->child("pair");
	IF_T(pPair->empty());

	PCREGIST_PAIR P;
	int i = 0;
	while (1)
	{
		Kiss* pP = pPair->child(i++);
		if(pP->empty())break;

		P.init();
		string n;

		n = "";
		pP->v("source", &n);
		P.m_pSource = (_PCbase*) (pK->getInst(n));
		IF_Fl(!P.m_pSource, n + ": not found");

		n = "";
		pP->v("target", &n);
		P.m_pTarget = (_PCbase*) (pK->getInst(n));
		IF_Fl(!P.m_pTarget, n + ": not found");

		n = "";
		pP->v("_PCtransform", &n);
		P.m_pTf = (_PCtransform*) (pK->getInst(n));
		IF_Fl(!P.m_pTf, n + ": not found");

		m_vpPair.push_back(P);
	}

	IF_F(m_vpPair.empty());

	return true;
}

bool _PCregistration::start(void)
{
	IF_F(!this->_ThreadBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _PCregistration::check(void)
{
	return 0;
}

void _PCregistration::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateRegistration();

		this->autoFPSto();
	}
}

void _PCregistration::updateRegistration(void)
{
	IF_(check() < 0);

	int nPC = m_vpPair.size();
	for(int i=0; i<nPC; i++)
	{
		PCREGIST_PAIR* p = &m_vpPair[i];

		PointCloud pcSource = *p->m_pSource->getPC();
		PointCloud pcTarget = *p->m_pTarget->getPC();

		registration::RegistrationResult rr = open3d::registration::RegistrationICP(
												pcSource,
												pcTarget,
												m_thr,
												Eigen::Matrix4d::Identity(),
												open3d::registration::TransformationEstimationPointToPoint()
												);

		Eigen::Matrix4d_u m = p->m_pTf->getTranslationMatrix(p->m_iMt) * rr.transformation_;
		p->m_pTf->setTranslationMatrix(p->m_iMt, m);
	}

}

void _PCregistration::draw(void)
{
	this->_ThreadBase::draw();
}

}
#endif
#endif
