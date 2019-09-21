/*
 * _TrackerBase.h
 *
 *  Created on: Aug 28, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Tracker__TrackerBase_H_
#define OpenKAI_src_Tracker__TrackerBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Detector/_DetectorBase.h"
#include "../Vision/_VisionBase.h"

namespace kai
{

enum TRACK_STATE
{
	track_init,
	track_update,
	track_stop
};

class _TrackerBase: public _ThreadBase
{
public:
	_TrackerBase();
	virtual
	~_TrackerBase();

	bool init(void* pKiss);
	void update(void);
	int check(void);

	virtual bool draw(void);
	virtual bool console(int& iY);

	virtual void createTracker(void);
	virtual bool startTrack(vFloat4& bb);
	virtual void stopTrack(void);
	TRACK_STATE trackState(void);
	vFloat4* getBB(void);

public:
	_VisionBase* m_pV;
	Rect2d m_rBB;
	vFloat4 m_bb;
	float m_margin;

	Rect2d	m_newBB;
	uint64_t	m_iSet;
	uint64_t	m_iInit;

	string m_trackerType;
	TRACK_STATE	m_trackState;

};

}
#endif
