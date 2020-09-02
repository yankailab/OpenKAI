/*
 * PointCloudBase.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud__PointCloudBase_H_
#define OpenKAI_src_PointCloud__PointCloudBase_H_

#include "../Base/_ThreadBase.h"

#ifdef USE_OPEN3D
using namespace open3d;
using namespace open3d::geometry;
using namespace open3d::visualization;

namespace kai
{

enum POINTCLOUD_TYPE
{
	pointCloud_unknown,
	pointCloud_viewer,
	pointCloud_realsense,
	pointCloud_livox,
};

class _PointCloudBase: public _ThreadBase
{
public:
	_PointCloudBase();
	virtual ~_PointCloudBase();

	virtual bool init(void* pKiss);
	virtual void draw(void);

	virtual bool open(void);
	virtual bool isOpened(void);
	virtual void close(void);
	virtual int size(void);
	virtual POINTCLOUD_TYPE getType(void);

	virtual void transform(void);

public:
    PointCloud* m_pPC;
	vFloat3 m_vT;	//translation
	vFloat3 m_vR;	//rotation
	vFloat2 m_vRz;	//z region

	bool m_bOpen;
	POINTCLOUD_TYPE m_type;
};

}
#endif
#endif
