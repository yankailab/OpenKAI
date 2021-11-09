/*
 * _PCdownSample.cpp
 *
 *  Created on: Feb 7, 2021
 *      Author: yankai
 */

#ifdef USE_OPEN3D
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

bool _PCdownSample::init(void *pKiss)
{
	IF_F(!_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

    pK->v("rVoxel", &m_rVoxel);

	return true;
}

bool _PCdownSample::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _PCdownSample::check(void)
{
	NULL__(m_pInCtx.m_pPCB, -1);

	return _PCbase::check();
}

void _PCdownSample::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		updateFilter();

		m_pT->autoFPSto();
	}
}

void _PCdownSample::updateFilter(void)
{
	IF_(check()<0);

	// PointCloud* pOut = m_sPC.next();
	// PointCloud pcIn;
    // m_pPCB->getPC(&pcIn);
    
    // *pOut = *pcIn.VoxelDownSample(m_rVoxel);    
}

}
#endif
