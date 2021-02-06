/*
 * _PCdownSample.h
 *
 *  Created on: Feb 7, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCdownSample_H_
#define OpenKAI_src_PointCloud_PCdownSample_H_

#include "../../Base/common.h"

#ifdef USE_OPEN3D
#include "../_PCbase.h"

namespace kai
{

class _PCdownSample: public _PCbase
{
public:
	_PCdownSample();
	virtual ~_PCdownSample();

	bool init(void* pKiss);
	bool start(void);
	int check(void);

private:
	void update(void);
	void updateFilter(void);
	static void* getUpdate(void* This)
	{
		((_PCdownSample *) This)->update();
		return NULL;
	}

public:
    float m_voxelSize;
};

}
#endif
#endif
