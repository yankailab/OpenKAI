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

	bool _PCcrop::init(const json &j)
	{
		IF_F(!this->_GeometryBase::init(j));

		const json &jF = j.at("vFilter");
		IF_F(!jF.is_object());

		for (auto it = jF.begin(); it != jF.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			POINTCLOUD_VOL v;
			v.init();
			jVar(Ji, "type", (int&)v.m_type);
			jVar(Ji, "bInside", v.m_bInside);
			jVec<float>(Ji, "vX", v.m_vX);
			jVec<float>(Ji, "vY", v.m_vY);
			jVec<float>(Ji, "vZ", v.m_vZ);
			jVec<float>(Ji, "vC", v.m_vC);
			jVec<float>(Ji, "vR", v.m_vR);
			m_vFilter.push_back(v);
		}

		return true;
	}

	bool _PCcrop::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _PCcrop::check(void)
	{
		//		NULL_F(m_pInCtx.m_pPCB);

		return _GeometryBase::check();
	}

	void _PCcrop::update(void)
	{
		while (m_pT->bAlive())
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
