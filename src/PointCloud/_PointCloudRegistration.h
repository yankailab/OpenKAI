/*
 * _PointCloudRegistration.h
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_PointCloudRegistration_H_
#define OpenKAI_src_Vision_PointCloudRegistration_H_

#include "../Base/common.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D
#include "_PointCloudBase.h"

namespace kai
{

struct PCREGIST_PAIR
{
	_PointCloudBase* m_pSource;
	_PointCloudBase* m_pTarget;
	float m_thr;

	void init(void)
	{
		m_pSource = NULL;
		m_pTarget = NULL;
	}

};

class _PointCloudRegistration: public _PointCloudBase
{
public:
	_PointCloudRegistration();
	virtual ~_PointCloudRegistration();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void draw(void);

private:
	void updateRegistration(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_PointCloudRegistration *) This)->update();
		return NULL;
	}

public:
	vector<PCREGIST_PAIR> m_vpPair;

};

}
#endif
#endif
#endif
