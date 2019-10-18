/*
 * Path.h
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Navigation__Path_H_
#define OpenKAI_src_Navigation__Path_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "Coordinate.h"

namespace kai
{

class _Path: public _ThreadBase
{
public:
	_Path(void);
	virtual ~_Path();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);

	void startRec(void);
	void pauseRec(void);
	void resumeRec(void);
	void stopRec(void);


private:
	void updateGPS(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Path *) This)->update();
		return NULL;
	}

public:
	Coordinate m_coord;
	bool	m_bRecord;
	double	m_dInterval;

	LL_POS m_pos;
	LL_POS m_lastPos;
	vector<LL_POS> m_vWP;

	float	m_showScale;

};

}
#endif
