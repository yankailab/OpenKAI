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

	int _PCdownSample::init(void *pKiss)
	{
		CHECK_(_GeometryBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("rVoxel", &m_rVoxel);

		return OK_OK;
	}

	int _PCdownSample::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _PCdownSample::check(void)
	{
		//	NULL__(m_pInCtx.m_pPCB, -1);

		return _GeometryBase::check();
	}

	void _PCdownSample::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateFilter();

			m_pT->autoFPSto();
		}
	}

	void _PCdownSample::updateFilter(void)
	{
		IF_(check() != OK_OK);

		// PointCloud* pOut = m_sPC.next();
		// PointCloud pcIn;
		// m_pPCB->getPC(&pcIn);

		// *pOut = *pcIn.VoxelDownSample(m_rVoxel);
	}

}
