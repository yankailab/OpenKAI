/*
 * PointCloudBase.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud__PointCloudViewer_H_
#define OpenKAI_src_PointCloud__PointCloudViewer_H_

#include "_PointCloudBase.h"

#ifdef USE_OPEN3D
using namespace open3d;
using namespace open3d::geometry;
using namespace open3d::visualization;

namespace kai
{

class _PointCloudViewer: public _PointCloudBase
{
public:
	_PointCloudViewer();
	virtual ~_PointCloudViewer();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);
	virtual POINTCLOUD_TYPE getType(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_PointCloudViewer *) This)->update();
		return NULL;
	}

public:
	Visualizer m_vis;
	vInt2 m_vWinSize;
	string m_fName;

};

}
#endif
#endif
