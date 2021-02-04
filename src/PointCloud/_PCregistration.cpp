/*
 * _PCregistration.cpp
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#include "_PCregistration.h"

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
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _PCregistration::check(void)
{
	return _PCbase::check();
}

void _PCregistration::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		updateRegistration();

		m_pT->autoFPSto();
	}
}

void _PCregistration::updateRegistration(void)
{
	IF_(check() < 0);

	int nPC = m_vpPair.size();
	for(int i=0; i<nPC; i++)
	{
		PCREGIST_PAIR* p = &m_vpPair[i];

		PointCloud pcSource;
        p->m_pSource->getPC(&pcSource);
		PointCloud pcTarget;
        p->m_pTarget->getPC(&pcTarget);

		pipelines::registration::RegistrationResult rr = pipelines::registration::RegistrationICP(
												pcSource,
												pcTarget,
												m_thr,
												Eigen::Matrix4d::Identity(),
												pipelines::registration::TransformationEstimationPointToPoint()
												);

		Eigen::Matrix4d_u m = p->m_pTf->getTranslationMatrix(p->m_iMt) * rr.transformation_;
		p->m_pTf->setTranslationMatrix(p->m_iMt, m);
	}

}

void _PCregistration::draw(void)
{
	this->_ModuleBase::draw();
}

}
#endif
