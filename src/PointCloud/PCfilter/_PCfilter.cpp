/*
 * _PCfilter.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#include "_PCfilter.h"

#ifdef USE_OPEN3D

namespace kai
{

_PCfilter::_PCfilter()
{
}

_PCfilter::~_PCfilter()
{
}

bool _PCfilter::init(void *pKiss)
{
	IF_F(!_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	Kiss *pF = pK->child("vFilter");
	int i = 0;
	while (1)
	{
		Kiss *pA = pF->child(i++);
		if (pA->empty())
			break;

		POINTCLOUD_VOL v;
		v.init();
		pA->v("type", (int*)&v.m_type);
		pA->v("bInside", &v.m_bInside);
		pA->v("vX", &v.m_vX);
		pA->v("vY", &v.m_vY);
		pA->v("vZ", &v.m_vZ);
		pA->v("vC", &v.m_vC);
		pA->v("vR", &v.m_vR);
		m_vFilter.push_back(v);
	}

	return true;
}

bool _PCfilter::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _PCfilter::check(void)
{
	NULL__(m_pPCB, -1);

	return _PCbase::check();
}

void _PCfilter::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		updateFilter();
		m_sPC.update();

		m_pT->autoFPSto();
	}
}

void _PCfilter::updateFilter(void)
{
	IF_(check()<0);

	PointCloud* pOut = m_sPC.next();
	pOut->points_.clear();
	pOut->colors_.clear();
	pOut->normals_.clear();

	PointCloud pcIn;
    m_pPCB->getPC(&pcIn);
	int nP = pcIn.points_.size();
	for (int i = 0; i < nP; i++)
	{
		Eigen::Vector3d vP = pcIn.points_[i];
		IF_CONT(!bFilter(vP));

		pOut->points_.push_back(vP);
		pOut->colors_.push_back( pcIn.colors_[i]);
	}
}

bool _PCfilter::bFilter(Eigen::Vector3d& vP)
{
	for (POINTCLOUD_VOL v : m_vFilter)
	{
		vFloat3 vf;
		vf = vP;
		IF_CONT(v.bValid(vf));
		return false;
	}

	return true;
}

}
#endif
