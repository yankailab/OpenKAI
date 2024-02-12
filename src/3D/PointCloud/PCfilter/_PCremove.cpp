/*
 * _PCremove.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#include "_PCremove.h"

namespace kai
{

_PCremove::_PCremove()
{
    m_nP = 16;
    m_r = 0.05;
}

_PCremove::~_PCremove()
{
}

bool _PCremove::init(void *pKiss)
{
	IF_F(!_GeometryBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

    pK->v("nP", &m_nP);
    pK->v("r", &m_r);

	return true;
}

bool _PCremove::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _PCremove::check(void)
{
//	NULL__(m_pInCtx.m_pPCB, -1);

	return _GeometryBase::check();
}

void _PCremove::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		updateFilter();

		m_pT->autoFPSto();
	}
}

void _PCremove::updateFilter(void)
{
	IF_(check()<0);

	// PointCloud pcIn;
    // m_pPCB->getPC(&pcIn);

    // auto[pc, vP] = pcIn.RemoveStatisticalOutliers(20,2.0);//m_nP, m_r);

    // PointCloud* pOut = m_sPC.next();
    // *pOut = *pc;
}

}
