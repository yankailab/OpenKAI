/*
 * _SLAM.cpp
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#include "_SLAM.h"

namespace kai
{

_SLAM::_SLAM()
{
	m_pObs = NULL;
	m_pMN = NULL;
	m_pSF40 = NULL;
	m_pMavlink = NULL;

	m_gridDim.init();
}

_SLAM::~_SLAM()
{

}

bool _SLAM::init(void* pKiss)
{
	IF_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("dimX", &m_gridDim.m_x));
	F_INFO(pK->v("dimY", &m_gridDim.m_y));
	F_INFO(pK->v("dimZ", &m_gridDim.m_z));

	int i,j;

	// create grid
	m_grid.resize(m_gridDim.m_x);
	for (i = 0; i < m_gridDim.m_x; i++)
	{
		m_grid[i].resize(m_gridDim.m_y);

		for (j = 0; j < m_gridDim.m_y; j++)
			m_grid[i][j].resize(m_gridDim.m_z);
	}

	return true;
}

bool _SLAM::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("_Obstacle", &iName));
	m_pObs = (_Obstacle*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_MatrixNet", &iName));
	m_pMN = (_MatrixNet*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Lightware_SF40", &iName));
	m_pSF40 = (_Lightware_SF40*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Mavlink", &iName));
	m_pMavlink= (_Mavlink*) (pK->root()->getChildInstByName(&iName));


	return true;
}

bool _SLAM::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _SLAM::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _SLAM::detect(void)
{
}

bool _SLAM::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->getCMat();
	IF_F(pMat->empty());

	return true;
}

}
