/*
 * _PCmerge.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PCmerge.h"

namespace kai
{

	_PCmerge::_PCmerge()
	{
	}

	_PCmerge::~_PCmerge()
	{
	}

	bool _PCmerge::loadConfig(void)
	{
		IF_F(!this->_GeometryBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "rVoxel", m_rVoxel);

		return true;
	}

	bool _PCmerge::link(void)
	{
		IF_F(!this->_GeometryBase::link());
		const json &j = *m_pJ;

		vector<string> vPCB;
		jKv(j, "vPCbase", vPCB);
		IF_F(vPCB.empty());

		for (string p : vPCB)
		{
			_GeometryBase *pPCB = (_GeometryBase *)(m_pM->findModule(p));
			IF_CONT(!pPCB);

			m_vpGB.push_back(pPCB);
		}
		IF_F(m_vpGB.empty());

		return true;
	}

	bool _PCmerge::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _PCmerge::check(void)
	{
		return this->_GeometryBase::check();
	}

	void _PCmerge::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateMerge();
		}
	}

	void _PCmerge::updateMerge(void)
	{
		IF_(!check());

		// read all inputs into one ring
		// for (_GeometryBase *pPCB : m_vpGB)
		{
			//		m_ring.readSrc(pPCB->getRing());
		}
	}

}
