/*
 * PointCloudBase.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud__PointCloudBase_H_
#define OpenKAI_src_PointCloud__PointCloudBase_H_

#include "../Base/_ThreadBase.h"
#include "_PCviewer.h"

#ifdef USE_OPEN3D
using namespace open3d;
using namespace open3d::geometry;
using namespace open3d::visualization;

namespace kai
{


class _PointCloudBase: public _ThreadBase
{
public:
	_PointCloudBase();
	virtual ~_PointCloudBase();

	virtual bool init(void* pKiss);
	virtual void draw(void);
	virtual int size(void);
	virtual int check(void);

	virtual PointCloud* getPC(void);

public:
	_PointCloudBase* m_pPCB;
	vSwitch<PointCloud> m_sPC;

	_PCviewer* m_pViewer;
};

}
#endif
#endif
