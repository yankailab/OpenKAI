/*
 * _RealSensePC.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RealSensePC_H_
#define OpenKAI_src_Vision_RealSensePC_H_

#include "../Base/common.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D
#ifdef USE_REALSENSE
#include "../Vision/_RealSense.h"
#include "_PointCloudViewer.h"

namespace kai
{

class _RealSensePC: public _PointCloudBase
{
public:
	_RealSensePC();
	virtual ~_RealSensePC();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);
	int check(void);
	void draw(void);

private:
	void updatePC(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_RealSensePC *) This)->update();
		return NULL;
	}

public:
	_PointCloudViewer* m_pViewer;
	_RealSense* m_pRS;
    rs2::pointcloud m_rsPC;
    rs2::points m_rsPoints;
    PointCloud* m_pPC;

	pthread_mutex_t m_mutex;

    Image m_imgD;
    Image m_imgRGB;

};

}
#endif
#endif
#endif
#endif
