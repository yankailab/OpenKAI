/*
 * _PCfile.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#include "_PCfile.h"

namespace kai
{

	_PCfile::_PCfile()
	{
	}

	_PCfile::~_PCfile()
	{
	}

	bool _PCfile::init(const json &j)
	{
		IF_F(!this->_PointCloud::init(j));

		jKv(j, "vfName", m_vfName);
		open();

		return true;
	}

	bool _PCfile::open(void)
	{
		IF_F(m_vfName.empty());

		m_pc.Clear();
		PointCloud pc;
		for (string f : m_vfName)
		{
			pc.Clear();
			//	io::ReadPointCloudOption ro;
			IF_CONT(!io::ReadPointCloud(f, pc));
			m_pc += pc;
			LOG_I("File: " + f + ", Npoints: " + i2str(pc.points_.size()));
		}

		clear();

        atomicFrom();

		for (size_t i = 0; i < m_pc.points_.size(); i++)
		{
			Vector3f vC = {1,1,1};
			if (i < m_pc.colors_.size())
				vC = m_pc.colors_[i].cast<float>();

			add(m_pc.points_[i], vC);
		}

        atomicTo();

		return true;
	}

	bool _PCfile::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _PCfile::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();
		}
	}

}
