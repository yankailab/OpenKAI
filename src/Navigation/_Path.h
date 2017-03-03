/*
 * Path.h
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#ifndef SRC_NAVIGATION_PATH_H_
#define SRC_NAVIGATION_PATH_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Navigation/_GPS.h"

namespace kai
{

class _Path: public _ThreadBase
{
public:
	_Path(void);
	virtual ~_Path();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	void startRecord(void);
	void stopRecord(void);
	void compress(void);
	void reset(void);
	UTM_POS* getCurrentPos(void);
	UTM_POS* getLastWayPoint(void);
	UTM_POS* getWayPoint(int iWP);

private:
	void updateGPS(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Path *) This)->update();
		return NULL;
	}

public:
	_GPS*	m_pGPS;

	bool	m_bRecord;
	double	m_dInterval;
	double	m_distCompress;
	vector<UTM_POS> m_vWP;
	UTM_POS m_lastWP;
	UTM_POS m_baseWP;

	double	m_showScale;

};

}
#endif
