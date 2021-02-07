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
#include <open3d/pipelines/registration/FastGlobalRegistration.h>

using namespace open3d::pipelines::registration;

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
	std::shared_ptr<Feature> preprocess(PointCloud& pc);
    RegistrationResult fastGlobalRegistration(void);
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
	int m_maxNNnormal;
	int m_maxNNfpfh;

	_PCbase* m_pSource;
	_PCbase* m_pTarget;
    RegistrationResult m_RR;
    
    _PCtransform* m_pTf;
	int m_iMt;

};

}
#endif
#endif
