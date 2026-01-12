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

		const json &jf = j.at("vFilter");
		IF_F(!jf.is_object());

		for (auto iF = jf.begin(); iF != jf.end(); iF++)
		{
			const json &jFi = iF.value();
			IF_CONT(!jFi.is_object());

			POINTCLOUD_VOL v;
			v.init();
			v.m_type = (POINTCLOUD_VOL_TYPE)jFi.value<int>("type", 0);
			v.m_bInside = jFi.value("bInside", true);
			v.m_vX = jFi.value("vX", vector<float>{});
			v.m_vY = jFi.value("vY", vector<float>{});
			v.m_vZ = jFi.value("vZ", vector<float>{});
			v.m_vC = jFi.value("vC", vector<float>{});
			v.m_vR = jFi.value("vR", vector<float>{});
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
