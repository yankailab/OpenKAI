/*
 * _ROITracker.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_ROITRACKER_H_
#define SRC_ROITRACKER_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Stream/_Stream.h"

using namespace cv;
using namespace cv::cuda;
using namespace std;

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
	_Stream* m_pStream;
	Rect2d m_ROI;
	Rect2d m_newROI;
	Frame* m_pFrame;
	bool m_bTracking;

	Ptr<Tracker> m_pTracker;
};

} /* namespace kai */

#endif /* SRC_CAMDENSEFLOW_H_ */
