/*
 * _ORB_SLAM2.h
 *
 *  Created on: Jul 18, 2017
 *      Author: yankai
 */

#ifndef SRC_SLAM_ORB_SLAM2_H_
#define SRC_SLAM_ORB_SLAM2_H_

#include "../Base/common.h"
#include "../Vision/_VisionBase.h"
#include <System.h>

namespace kai
{

class _ORB_SLAM2: public _ThreadBase
{
public:
	_ORB_SLAM2();
	virtual ~_ORB_SLAM2();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ORB_SLAM2*) This)->update();
		return NULL;
	}

public:
	int	m_width;
	int m_height;

	_VisionBase*	m_pVision;
	ORB_SLAM2::System* m_pOS;
	Mat m_pose;
	Frame* m_pFrame;
	uint64_t m_tStartup;

};

}

#endif
