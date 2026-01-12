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

		m_rVoxel = j.value("rVoxel", 0.1);

        return true;
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
			m_pT->autoFPS();

			updateFilter();

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
