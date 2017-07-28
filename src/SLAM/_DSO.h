/*
 * _DSO.h
 *
 *  Created on: Jul 26, 2017
 *      Author: yankai
 */

#ifndef SRC_SLAM_DSO_H_
#define SRC_SLAM_DSO_H_

#include "../Base/common.h"

#ifdef USE_DSO

#include "../Vision/_VisionBase.h"
#include "DSOwrap.h"
#include "Eigen/Eigen"
#include <opencv2/core/eigen.hpp>

namespace kai
{

class _DSO: public _ThreadBase
{
public:
	_DSO();
	virtual ~_DSO();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	bool bTracking(void);
	void reset(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DSO*) This)->update();
		return NULL;
	}

public:
	int	m_width;
	int m_height;
	bool m_bViewer;
	int m_frameID;

	_VisionBase*	m_pVision;
	Frame* m_pFrame;
	DSOwrap* m_pDSO;

};

}

#endif
#endif
