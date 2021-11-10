/*
 * _PCcrop.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#include "_PCcrop.h"

namespace kai
{

	_PCcrop::_PCcrop()
	{
	}

	_PCcrop::~_PCcrop()
	{
	}

	bool _PCcrop::init(void *pKiss)
	{
		IF_F(!_GeometryBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		Kiss *pF = pK->child("vFilter");
		int i = 0;
		while (1)
		{
			Kiss *pA = pF->child(i++);
			if (pA->empty())
				break;

			POINTCLOUD_VOL v;
			v.init();
			pA->v("type", (int *)&v.m_type);
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

	bool _PCcrop::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _PCcrop::check(void)
	{
		NULL__(m_pInCtx.m_pPCB, -1);

		return _GeometryBase::check();
	}

	void _PCcrop::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateFilter();

			m_pT->autoFPSto();
		}
	}

	void _PCcrop::updateFilter(void)
	{
		IF_(check() < 0);

		// PointCloud* pOut = m_sPC.next();
		// PointCloud pcIn;
		// m_pPCB->getPC(&pcIn);
		// int nP = pcIn.points_.size();
		// for (int i = 0; i < nP; i++)
		// {
		// 	Eigen::Vector3d vP = pcIn.points_[i];
		// 	IF_CONT(!bFilter(vP));

		// 	pOut->points_.push_back(vP);
		// 	pOut->colors_.push_back( pcIn.colors_[i]);
		// }
	}

	bool _PCcrop::bFilter(Eigen::Vector3d &vP)
	{
		for (POINTCLOUD_VOL v : m_vFilter)
		{
			vFloat3 vf;
			vf = vP.cast<float>();
			IF_CONT(v.bValid(vf));
			return false;
		}

		return true;
	}

}
