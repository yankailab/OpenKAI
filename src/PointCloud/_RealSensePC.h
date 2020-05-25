/*
 * _RealSensePC.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RealSensePC_H_
#define OpenKAI_src_Vision_RealSensePC_H_

#include "../Base/common.h"
#include "../Base/open3d.h"
#include "../Vision/_RealSense.h"

#ifdef USE_REALSENSE
#ifdef USE_OPEN3D
#include "_PointCloudBase.h"

namespace kai
{

class _RealSensePC: public _PointCloudBase
{
public:
	_RealSensePC();
	virtual ~_RealSensePC();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	bool open(void);
	void close(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_RealSensePC *) This)->update();
		return NULL;
	}

public:
	_RealSense* m_pRS;
    rs2::pointcloud m_rsPC;

};

}
#endif
#endif
#endif
