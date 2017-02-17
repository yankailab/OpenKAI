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
#include "../Stream/_ZED.h"

namespace kai
{

struct WAY_POINT
{
	vDouble3	m_p;
	double		m_hdg;

	void init(void)
	{
		m_p.init();
		m_hdg = 0.0;
	}
};

class _Path: public _ThreadBase
{
public:
	_Path(void);
	virtual ~_Path();

	bool init(void* pKiss);
	bool link(void);
	bool draw(void);

	void startRecord(void);
	void stopRecord(void);
	void reset(void);
	WAY_POINT getLastWayPoint(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Path *) This)->update();
		return NULL;
	}

public:
	_ZED*	m_pZed;

	bool	m_bRecord;
	double m_dInterval;
	vector<WAY_POINT> m_vWP;
	WAY_POINT m_lastWP;
	WAY_POINT m_baseWP;

	double	m_showScale;

};

}
#endif
