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
#include "_PCtransform.h"

namespace kai
{

struct PCREGIST_PAIR
{
	_PCbase* m_pSource;
	_PCbase* m_pTarget;
	_PCtransform* m_pTf;
	int m_iMt;

	void init(void)
	{
		m_pSource = NULL;
		m_pTarget = NULL;
		m_pTf = NULL;
		m_iMt = 0;
	}
};

class _PCregistration: public _PCbase
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

	float m_thr;	//ICP threshold
	int m_nMinP;	//minimum number of points needed for registration

};

}
#endif
#endif
#endif
