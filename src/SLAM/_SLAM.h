/*
 * _SLAM.h
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#ifndef SRC_SLAM__SLAM_H_
#define SRC_SLAM__SLAM_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Sensor/_Lightware_SF40.h"
#include "../Protocol/_Mavlink.h"
#include "../Automaton/_Automaton.h"
#include "../AI/_MatrixNet.h"
#include "../Navigation/_Obstacle.h"

using std::vector;

namespace kai
{

struct GRID_CELL
{
	uint16_t	m_iClass;
	uint64_t	m_tLastUpdate;
	uint32_t	m_data;
};

class _SLAM: public _ThreadBase
{
public:
	_SLAM(void);
	virtual ~_SLAM();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_SLAM *) This)->update();
		return NULL;
	}

public:
	_Obstacle* m_pObs;
	_MatrixNet* m_pMN;
	_Lightware_SF40* m_pSF40;
	_Mavlink* m_pMavlink;

	vInt3	m_gridDim;
	vector<vector<vector<GRID_CELL> > > m_grid;



};

}

#endif
