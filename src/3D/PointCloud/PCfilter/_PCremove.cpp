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

	bool _PCremove::init(const json &j)
	{
		IF_F(!this->_GeometryBase::init(j));

		jKv(j, "nP", m_nP);
		jKv(j, "r", m_r);

		return true;
	}

	bool _PCremove::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _PCremove::check(void)
	{
		//	NULL_F(m_pInCtx.m_pPCB);

		return _GeometryBase::check();
	}

	void _PCremove::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateFilter();
		}
	}

	void _PCremove::updateFilter(void)
	{
		IF_(!check());

		// PointCloud pcIn;
		// m_pPCB->getPC(&pcIn);

		// auto[pc, vP] = pcIn.RemoveStatisticalOutliers(20,2.0);//m_nP, m_r);

		// PointCloud* pOut = m_sPC.next();
		// *pOut = *pc;
	}

}
