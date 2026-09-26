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

	bool _PCcrop::loadConfig(void)
	{
		IF_F(!this->_GeometryBase::loadConfig());
		const json &j = *m_pJ;

		const json *pJF = jK(j, "vFilter");
		IF_F(!pJF || !pJF->is_object());
		const json &jF = *pJF;

		for (auto it = jF.begin(); it != jF.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			POINTCLOUD_VOL v;
			v.init();
			jKv(Ji, "type", (int&)v.m_type);
			jKv(Ji, "bInside", v.m_bInside);
			jKv<float>(Ji, "vX", v.m_vX);
			jKv<float>(Ji, "vY", v.m_vY);
			jKv<float>(Ji, "vZ", v.m_vZ);
			jKv<float>(Ji, "vC", v.m_vC);
			jKv<float>(Ji, "vR", v.m_vR);
			m_vFilter.push_back(v);
		}

		return true;
	}

	bool _PCcrop::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _PCcrop::check(void)
	{
		//		NULL_F(m_pInCtx.m_pPCB);

		return _GeometryBase::check();
	}

	void _PCcrop::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateFilter();
		}
	}

	void _PCcrop::updateFilter(void)
	{
		IF_(!check());

		// PointCloud* pOut = m_sPC.next();
		// PointCloud pcIn;
		// m_pPCB->getPC(&pcIn);
		// int nP = pcIn.points_.size();
		// for (int i = 0; i < nP; i++)
		// {
		// 	Vector3d vP = pcIn.points_[i];
		// 	IF_CONT(!bFilter(vP));

		// 	pOut->points_.push_back(vP);
		// 	pOut->colors_.push_back( pcIn.colors_[i]);
		// }
	}

	bool _PCcrop::bFilter(Vector3d &vP)
	{
		for (POINTCLOUD_VOL v : m_vFilter)
		{
			Vector3f vf = Vector3f::Zero();
			vf = vP.cast<float>();
			IF_CONT(v.bValid(vf));
			return false;
		}

		return true;
	}

}
