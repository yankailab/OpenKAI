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
	}

	_PCremove::~_PCremove()
	{
	}

	bool _PCremove::loadConfig(void)
	{
		IF_F(!this->_GeometryBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "nP", m_nP);
		jKv(j, "r", m_r);

		return true;
	}

	bool _PCremove::saveConfig(bool bExport)
	{
		if (!_GeometryBase::saveConfig(false))
		{
			return false;
		}

		json &j = *m_pJ;
		j["nP"] = m_nP;
		j["r"] = m_r;

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _PCremove::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _PCremove::check(void)
	{
		//	NULL_F(m_pInCtx.m_pPCB);

		return _GeometryBase::check();
	}

	void _PCremove::update(void)
	{
		while (m_pT->bRun())
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
