/*
 * _PCdownSample.cpp
 *
 *  Created on: Feb 7, 2021
 *      Author: yankai
 */

#include "_PCdownSample.h"

#ifdef USE_OPEN3D

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
	NULL__(m_pPCB, -1);

	return _PCbase::check();
}

void _PCdownSample::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		updateFilter();
        updatePC();

  		if(m_pViewer)
		{
			m_pViewer->updateGeometry(m_iV, m_sPC.prev());
		}

		m_pT->autoFPSto();
	}
}

void _PCdownSample::updateFilter(void)
{
	IF_(check()<0);

	PointCloud* pOut = m_sPC.next();
	pOut->points_.clear();
	pOut->colors_.clear();
	pOut->normals_.clear();

	PointCloud pcIn;
    m_pPCB->getPC(&pcIn);
    
    *pOut = *pcIn.VoxelDownSample(m_rVoxel);    
}

}
#endif
