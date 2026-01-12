/*
 * _PCremove.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#include "_PCremove.h"

namespace kai
{

	_PCremove::_PCremove()
	{
		m_nP = 16;
		m_r = 0.05;
	}

	_PCremove::~_PCremove()
	{
	}

    bool _PCremove::init(const json &j)
    {
        IF_F(!this->_GeometryBase::init(j));

		m_nP = j.value("nP", 16);
		m_r = j.value("r", 0.05);

        return true;
    }


	int _PCremove::init(void *pKiss)
	{
		CHECK_(_GeometryBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nP", &m_nP);
		pK->v("r", &m_r);

		return OK_OK;
	}

	int _PCremove::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _PCremove::check(void)
	{
		//	NULL__(m_pInCtx.m_pPCB, -1);

		return _GeometryBase::check();
	}

	void _PCremove::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateFilter();

		}
	}

	void _PCremove::updateFilter(void)
	{
		IF_(check() != OK_OK);

		// PointCloud pcIn;
		// m_pPCB->getPC(&pcIn);

		// auto[pc, vP] = pcIn.RemoveStatisticalOutliers(20,2.0);//m_nP, m_r);

		// PointCloud* pOut = m_sPC.next();
		// *pOut = *pc;
	}

}
