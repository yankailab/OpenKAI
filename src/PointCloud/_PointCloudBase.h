/*
 * PointCloudBase.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud__PointCloudBase_H_
#define OpenKAI_src_PointCloud__PointCloudBase_H_

#include "../Base/_ThreadBase.h"
#include "_PointCloudViewer.h"

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

	virtual bool open(void);
	virtual bool isOpened(void);
	virtual void close(void);
	virtual int size(void);
	virtual int check(void);

	virtual void setTranslation(vFloat3& vT);
	virtual void setRotation(vFloat3& vR);
	virtual void transform(void);

public:
	vSwitch<PointCloud> m_sPC;

	vFloat3 m_vT;	//translation
	vFloat3 m_vR;	//rotation
	vFloat2 m_vRz;	//z region

	bool m_bOpen;

	_PointCloudViewer* m_pViewer;
};

}
#endif
#endif
