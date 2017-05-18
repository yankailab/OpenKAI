/*
 * _LidarOdometry.h
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#ifndef OPENKAI_SRC_Odometry__LidarSLAM_H_
#define OPENKAI_SRC_Odometry__LidarSLAM_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Sensor/_Lightware_SF40.h"
#include "../Automaton/_Automaton.h"
#include "../DNN/_MatrixNet.h"
#include "../Sensor/_Obstacle.h"
#include "../Navigation/_GPS.h"

using std::vector;
#define MAX_CELL 1000
#define CELL_MIN_LEN 0.1

namespace kai
{

struct OBSTACLE_BOX
{
	vDouble4	m_fBBox;
	double		m_deg;

	void init(void)
	{
		m_fBBox.init();
		m_deg = 0.0;
	}
};

struct GRID_CELL
{
	int16_t		m_iClass;
	int32_t		m_data;
	uint64_t	m_tLastUpdate;

	void init(void)
	{
		m_iClass = -1;
		m_data = -1;
		m_tLastUpdate = 0;
	}
};

class _LidarOdometry: public _ThreadBase
{
public:
	_LidarOdometry(void);
	virtual ~_LidarOdometry();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

private:
	void expandGrid(vInt3* pPos);
	void updateGPS(void);
	void updateSF40(void);
	void updateObstacle(void);
	void updateMatrixNet(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_LidarOdometry *) This)->update();
		return NULL;
	}

public:
	_MatrixNet* m_pMN;
	_Lightware_SF40* m_pSF40;

	_GPS*		m_pGPS;
	vDouble3	m_attiRad;	//yaw, pitch, roll, heading
	double		m_hdgRad;
	double		m_initHdgRad;
	uint64_t	m_tLastMav;

	_Obstacle*	m_pObs;
	vDouble2	m_obsRange;
	vector<OBSTACLE_BOX> m_vObsBox;

	//X positive: Easting
	//Y positive: Northing
	//Z altitude
	vInt3		m_gridDim;
	vDouble3	m_cellLen;		//in meter
	vInt3		m_gridOrigin;	//init position
	vInt3		m_gridPos;
	deque<deque<deque<GRID_CELL> > > m_grid;

	uint64_t	m_tNow;


};

}

#endif
