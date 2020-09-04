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
#include "_PointCloudViewer.h"

namespace kai
{

class _PointCloudMerge: public _PointCloudBase
{
public:
	_PointCloudMerge();
	virtual ~_PointCloudMerge();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void draw(void);

private:
	void updatePC(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_PointCloudMerge *) This)->update();
		return NULL;
	}

public:
	_PointCloudViewer* m_pViewer;
	vector<_PointCloudBase*> m_vpPC;

	pthread_mutex_t m_mutex;

};

}
#endif
#endif
#endif
