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

		pK->v("fName", &m_fName);
		open();

		return OK_OK;
	}

	bool _PCfile::open(void)
	{
		IF_F(m_fName.empty());

		//	io::ReadPointCloudOption ro;
		IF_F(!io::ReadPointCloud(m_fName, m_pc));
		LOG_I("Read point cloud: " + i2str(m_pc.points_.size()));
		*m_sPC.get() = m_pc;

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
