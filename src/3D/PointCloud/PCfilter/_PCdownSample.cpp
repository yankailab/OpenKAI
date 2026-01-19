/*
 * _PCdownSample.cpp
 *
 *  Created on: Feb 7, 2021
 *      Author: yankai
 */

#include "_PCdownSample.h"

namespace kai
{

	_PCdownSample::_PCdownSample()
	{
		m_rVoxel = 0.1;
	}

	_PCdownSample::~_PCdownSample()
	{
	}

	bool _PCdownSample::init(const json &j)
	{
		IF_F(!this->_GeometryBase::init(j));

		jKv(j, "rVoxel", m_rVoxel);

		return true;
	}

	bool _PCdownSample::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _PCdownSample::check(void)
	{
		//	NULL_F(m_pInCtx.m_pPCB);

		return _GeometryBase::check();
	}

	void _PCdownSample::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateFilter();
		}
	}

	void _PCdownSample::updateFilter(void)
	{
		IF_(!check());

		// PointCloud* pOut = m_sPC.next();
		// PointCloud pcIn;
		// m_pPCB->getPC(&pcIn);

		// *pOut = *pcIn.VoxelDownSample(m_rVoxel);
	}

}
