/*
 * _PCregistICP.h
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCregistICP_H_
#define OpenKAI_src_PointCloud_PCregistICP_H_

#include "../Base/common.h"

#ifdef USE_OPEN3D
#include "_PCtransform.h"

namespace kai
{

class _PCregistICP: public _PCbase
{
public:
	_PCregistICP();
	virtual ~_PCregistICP();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void draw(void);

private:
	void updateRegistration(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		(( _PCregistICP *) This)->update();
		return NULL;
	}

public:
	float m_thr;	//ICP threshold
	int m_nMinP;	//minimum number of points needed for registration

	_PCbase* m_pSource;
	_PCbase* m_pTarget;
	_PCtransform* m_pTf;
	int m_iMt;

};

}
#endif
#endif
