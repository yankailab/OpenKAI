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
		m_rVoxel = 0.0;
	}

	_PCmerge::~_PCmerge()
	{
	}

	bool _PCmerge::init(const json &j)
	{
		IF_F(!this->_GeometryBase::init(j));

		jKv(j, "rVoxel", m_rVoxel);

		return true;
	}

	bool _PCmerge::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_GeometryBase::link(j, pM));

		vector<string> vPCB;
		jKv(j, "vPCbase", vPCB);
		IF_F(vPCB.empty());

		for (string p : vPCB)
		{
			_GeometryBase *pPCB = (_GeometryBase *)(pM->findModule(p));
			IF_CONT(!pPCB);

			m_vpGB.push_back(pPCB);
		}
		IF_F(m_vpGB.empty());

		return true;
	}

	bool _PCmerge::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _PCmerge::check(void)
	{
		return this->_GeometryBase::check();
	}

	void _PCmerge::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateMerge();
		}
	}

	void _PCmerge::updateMerge(void)
	{
		IF_(!check());

		// read all inputs into one ring
		for (_GeometryBase *pPCB : m_vpGB)
		{
			//		m_ring.readSrc(pPCB->getRing());
		}
	}

}
