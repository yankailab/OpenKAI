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

	bool _PCfile::init(void *pKiss)
	{
		IF_F(!_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("fName", &m_fName);
		open();

		return true;
	}

	bool _PCfile::open(void)
	{
		IF_F(m_fName.empty());

		//	io::ReadPointCloudOption ro;
		IF_F(!io::ReadPointCloud(m_fName, m_pc));
		*m_sPC.next() = m_pc;
		swapBuffer();
//		m_nPread = m_pc.points_.size();

		return true;
	}

	bool _PCfile::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _PCfile::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			*m_sPC.next() = m_pc;
			swapBuffer();

			m_pT->autoFPSto();
		}
	}

}
