/*
 * _PCregistration.h
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCregistration_H_
#define OpenKAI_src_PointCloud_PCregistration_H_

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
	int m_iMt;

	void init(void)
	{
		m_pSource = NULL;
		m_pTarget = NULL;
		m_thr = 0.02;
		m_iMt = 0;
	}
};

class _PCregistration: public _PointCloudBase
{
public:
	_PCregistration();
	virtual ~_PCregistration();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void draw(void);

private:
	void updateRegistration(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_PCregistration *) This)->update();
		return NULL;
	}

public:
	vector<PCREGIST_PAIR> m_vpPair;

};

}
#endif
#endif
#endif
