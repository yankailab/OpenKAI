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
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "nP", m_nP);
		jKv(j, "r", m_r);

		return true;
	}

	bool _PCremove::link(InstanceMgr *pM)
	{
		IF_F(!_ModuleBase::link(pM));

		string tfName;
		jKv(*m_pJ, "_TransformRigid", tfName);
		m_pTf = dynamic_cast<_TransformRigid *>(static_cast<_ModuleBase *>(pM->findModule(tfName)));
		IF_Le_F(!tfName.empty() && !m_pTf, "_TransformRigid not found: " + tfName);

		return true;
	}

	bool _PCremove::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));

		json &j = *m_pJ;
		j["nP"] = m_nP;
		j["r"] = m_r;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _PCremove::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _PCremove::check(void)
	{
		return _ModuleBase::check();
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

		// TODO: remove outliers from point-cloud snapshots.
	}

}
