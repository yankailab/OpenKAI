/*
 * _PCregistGlobal.h
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCregistGlobal_H_
#define OpenKAI_src_PointCloud_PCregistGlobal_H_

#include "../Base/common.h"

#ifdef USE_OPEN3D
#include "_PCtransform.h"

namespace kai
{

class _PCregistGlobal: public _PCbase
{
public:
	_PCregistGlobal();
	virtual ~_PCregistGlobal();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void draw(void);

private:
	void preprocess(PointCloud& pc);
	void updateRegistration(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		(( _PCregistGlobal *) This)->update();
		return NULL;
	}

public:
	float m_thr;	//ICP threshold
	int m_nMinP;	//minimum number of points needed for registration
	
	double m_voxelSize;
	int m_maxNN;

	_PCbase* m_pSource;
	_PCbase* m_pTarget;
	_PCtransform* m_pTf;
	int m_iMt;

};

}
#endif
#endif
