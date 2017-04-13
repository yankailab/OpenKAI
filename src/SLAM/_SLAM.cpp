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
	m_pGPS = NULL;

	m_attiRad.init();
	m_gridDim.init();
	m_gridOrigin.init();
	m_cellLen.init();
	m_gridPos.init();
	m_obsRange.init();

	m_tNow = 0;
	m_hdgRad = 0.0;
	m_initHdgRad = 0.0;
	m_tLastMav = 0;
}

_SLAM::~_SLAM()
{

}

bool _SLAM::init(void* pKiss)
{
	IF_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("dimX", &m_gridDim.x));
	F_INFO(pK->v("dimY", &m_gridDim.y));
	F_INFO(pK->v("dimZ", &m_gridDim.z));

	m_gridDim.x = constrain(m_gridDim.x, 1, MAX_CELL);
	m_gridDim.y = constrain(m_gridDim.y, 1, MAX_CELL);
	m_gridDim.z = constrain(m_gridDim.z, 1, MAX_CELL);

	m_grid.clear();
	m_gridOrigin.init();
	m_gridPos = m_gridOrigin;

	F_INFO(pK->v("lenX", &m_cellLen.x));
	F_INFO(pK->v("lenY", &m_cellLen.y));
	F_INFO(pK->v("lenZ", &m_cellLen.z));

	m_cellLen.x = constrain(m_cellLen.x, CELL_MIN_LEN, DBL_MAX);
	m_cellLen.y = constrain(m_cellLen.y, CELL_MIN_LEN, DBL_MAX);
	m_cellLen.z = constrain(m_cellLen.z, CELL_MIN_LEN, DBL_MAX);

	F_INFO(pK->v("obsRangeMin", &m_obsRange.x));
	F_INFO(pK->v("obsRangeMax", &m_obsRange.y));

	Kiss* pObs = pK->o("obstacleBox");
	IF_T(pObs->empty());

	m_vObsBox.clear();
	Kiss** pItr = pObs->getChildItr();
	int i = 0;
	while (pItr[i])
	{
		Kiss* pO = pItr[i++];

		OBSTACLE_BOX oBox;
		oBox.init();
		F_INFO(pO->v("left", &oBox.m_fBBox.x));
		F_INFO(pO->v("top", &oBox.m_fBBox.y));
		F_INFO(pO->v("right", &oBox.m_fBBox.z));
		F_INFO(pO->v("bottom", &oBox.m_fBBox.w));

		m_vObsBox.push_back(oBox);
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
	F_INFO(pK->v("_GPS", &iName));
	m_pGPS= (_GPS*) (pK->root()->getChildInstByName(&iName));

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

		m_tNow = get_time_usec();

		updateGPS();
		updateSF40();
		updateObstacle();
		updateMatrixNet();

		this->autoFPSto();
	}
}

void _SLAM::updateGPS(void)
{
	NULL_(m_pGPS);
	_Mavlink* pMavlink = m_pGPS->m_pMavlink;
	NULL_(pMavlink);

	m_attiRad.x = (double)pMavlink->m_msg.attitude.yaw;
	m_attiRad.y = (double)pMavlink->m_msg.attitude.pitch;
	m_attiRad.z = (double)pMavlink->m_msg.attitude.roll;
	m_hdgRad = ((double)pMavlink->m_msg.global_position_int.hdg) * 0.01;

	if(m_tLastMav == 0)
	{
		m_initHdgRad = m_hdgRad;
	}

	m_tLastMav = pMavlink->m_msg.time_stamps.global_position_int;

}

void _SLAM::updateSF40(void)
{
	NULL_(m_pSF40);
}

void _SLAM::updateObstacle(void)
{
	NULL_(m_pObs);

	int i;
	for(i=0; i<m_vObsBox.size(); i++)
	{
		OBSTACLE_BOX oB = m_vObsBox[i];

		//TODO
//		double d = m_pObs->dist(&oB.m_fBBox, NULL);
		double d = 0;

		IF_CONT(d < m_obsRange.x);
		IF_CONT(d > m_obsRange.y);

		double dirRad = m_hdgRad + (oB.m_deg * DEG_RAD);
		double dE = d * sin(dirRad);
		double dN = d * cos(dirRad);

		vInt3 p;
		p.x = m_gridOrigin.x + m_gridPos.x + (dE / m_cellLen.x);
		p.y = m_gridOrigin.y + m_gridPos.y + (dE / m_cellLen.y);
		p.z = 0;

		expandGrid(&p);

		GRID_CELL* pCell = &m_grid[p.x][p.y][p.z];
		pCell->m_data = 1;
		pCell->m_tLastUpdate = m_tNow;
	}
}

void _SLAM::expandGrid(vInt3* pPos)
{
	NULL_(pPos);

	GRID_CELL newCell;
	newCell.init();
	deque<GRID_CELL> eY;
	eY.push_back(newCell);
	deque<deque<GRID_CELL> > eX;
	eX.push_back(eY);

	while(pPos->x >= m_grid.size())
	{
		m_grid.push_back(eX);
	}

	while(pPos->x < 0)
	{
		m_grid.push_front(eX);
		m_gridOrigin.x++;
		pPos->x++;
	}

	deque<deque<GRID_CELL> >* pY = &m_grid[pPos->x];

	while(pPos->y >= pY->size())
	{
		pY->push_back(eY);
	}

	while(pPos->y < 0)
	{
		m_grid.push_front(eX);
		m_gridOrigin.x++;
		pPos->x++;
	}

}

void _SLAM::updateMatrixNet(void)
{
	NULL_(m_pMN);


}

bool _SLAM::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->getCMat();
	IF_F(pMat->empty());

	return true;
}

}
