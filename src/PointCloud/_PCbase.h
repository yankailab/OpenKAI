/*
 * PCbase.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud__PCbase_H_
#define OpenKAI_src_PointCloud__PCbase_H_

#include "../Base/_ThreadBase.h"
#include "_PCviewer.h"

#ifdef USE_OPEN3D
using namespace open3d;
using namespace open3d::geometry;
using namespace open3d::visualization;

namespace kai
{


class _PCbase: public _ThreadBase
{
public:
	_PCbase();
	virtual ~_PCbase();

	virtual bool init(void* pKiss);
	virtual int size(void);
	virtual int check(void);
	virtual void draw(void);

	virtual PointCloud* getPC(void);

public:
	_PCbase* m_pPCB;
	vSwitch<PointCloud> m_sPC;

	_PCviewer* m_pViewer;
	int m_iV;
};

}
#endif
#endif
