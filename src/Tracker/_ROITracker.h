/*
 * _ROITracker.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Tracker__ROITracker_H_
#define OpenKAI_src_Tracker__ROITracker_H_

#ifndef USE_OPENCV4TEGRA

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Vision/_VisionBase.h"

#ifndef USE_OPENCV4TEGRA

namespace kai
{

class _ROITracker: public _ThreadBase
{
public:
	_ROITracker();
	virtual ~_ROITracker();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

	void setROI(Rect2d roi);
	void tracking(bool bTracking);

private:
	void track(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ROITracker*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pStream;
	Rect2d m_ROI;
	Rect2d m_newROI;
	Frame* m_pFrame;
	bool m_bTracking;

	Ptr<Tracker> m_pTracker;
};

}

#endif
#endif
#endif
