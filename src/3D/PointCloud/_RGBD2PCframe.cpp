/*
 * _RGBD2PCframe.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_RGBD2PCframe.h"

namespace kai
{

	_RGBD2PCframe::_RGBD2PCframe()
	{
		m_pV = nullptr;
	}

	_RGBD2PCframe::~_RGBD2PCframe()
	{
	}

    bool _RGBD2PCframe::init(const json &j)
    {
        IF_F(!this->_PCframe::init(j));

        return true;
    }

    bool _RGBD2PCframe::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_PCframe::link(j, pM));

		string n = j.value("_RGBDbase", "");
		m_pV = (_RGBDbase *)(pM->findModule(n));
		NULL_F(m_pV);

        return true;
    }



	int _RGBD2PCframe::init(void *pKiss)
	{
		CHECK_(_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	int _RGBD2PCframe::link(void)
	{
		CHECK_(this->_PCframe::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_RGBDbase", &n);
		m_pV = (_RGBDbase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _RGBD2PCframe::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _RGBD2PCframe::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			getPointCloud();
		}
	}

	void _RGBD2PCframe::getPointCloud(void)
	{
		NULL_(m_pV);

        mutexLock();

        PointCloud *pPC = getNextBuffer();
        pPC->Clear();
        pPC->points_.clear();
        pPC->colors_.clear();

		int nP = m_pV->getPointCloud(this);

        mutexUnlock();

        swapBuffer();
	}
}
