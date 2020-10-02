/*
 * PointCloudBase.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud__PCviewer_H_
#define OpenKAI_src_PointCloud__PCviewer_H_

#include "../Base/_ThreadBase.h"

#ifdef USE_OPEN3D
using namespace open3d;
using namespace open3d::geometry;
using namespace open3d::visualization;

namespace kai
{

class _PCviewer: public _ThreadBase
{
public:
	_PCviewer();
	virtual ~_PCviewer();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

	void render(PointCloud* pPC);

private:
	void render(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_PCviewer *) This)->update();
		return NULL;
	}

public:
	Visualizer m_vis;
	vInt2 m_vWinSize;
	shared_ptr<TriangleMesh> m_pMcoordFrame;
	float m_fov;

	shared_ptr<PointCloud> m_spPC;
	pthread_mutex_t m_mutex;
};

}
#endif
#endif
