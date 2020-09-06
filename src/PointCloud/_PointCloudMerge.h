/*
 * _PointCloudMerge.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_PointCloudMerge_H_
#define OpenKAI_src_Vision_PointCloudMerge_H_

#include "../Base/common.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D
#include "_PointCloudBase.h"

namespace kai
{

class _PointCloudMerge: public _PointCloudBase
{
public:
	_PointCloudMerge();
	virtual ~_PointCloudMerge();

	bool init(void* pKiss);
	bool start(void);

private:
	void updatePC(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_PointCloudMerge *) This)->update();
		return NULL;
	}

public:
	vector<_Universe*> m_vpU;

};

}
#endif
#endif
#endif
