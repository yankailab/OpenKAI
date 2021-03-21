/*
 * _PCregistGlobal.h
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCregistGlobal_H_
#define OpenKAI_src_PointCloud_PCregistGlobal_H_

#ifdef USE_OPEN3D
#include "../PCfilter/_PCtransform.h"
#include <open3d/pipelines/registration/FastGlobalRegistration.h>
using namespace open3d::pipelines::registration;

namespace kai
{

class _PCregistGlobal: public _ModuleBase
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
    void updateRegistration(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		(( _PCregistGlobal *) This)->update();
		return NULL;
	}

public:
    double m_rNormal;
    double m_rFeature;
	int m_maxNNnormal;
	int m_maxNNfpfh;

	_PCframe* m_pSrc;
	_PCframe* m_pTgt;
    RegistrationResult m_RR;
	_PCtransform* m_pTf;
    double m_lastFit;
};

}
#endif
#endif
