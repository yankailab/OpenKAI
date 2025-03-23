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

	int _PCfile::init(void *pKiss)
	{
		CHECK_(_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->a("vfName", &m_vfName);
		open();

		return OK_OK;
	}

	bool _PCfile::open(void)
	{
		IF_F(m_vfName.empty());

		m_sPC.get()->Clear();
		PointCloud pc;
		for (string f : m_vfName)
		{
			//	io::ReadPointCloudOption ro;
			pc.Clear();
			IF_CONT(!io::ReadPointCloud(f, pc));
			LOG_I("File: " + f + ", Npoints: " + i2str(pc.points_.size()));
			*m_sPC.get() += pc;
		}

		return true;
	}

	int _PCfile::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _PCfile::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			writeSharedMem();
		}
	}

}
