/*
 * _PCregistCol.h
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCregistCol_H_
#define OpenKAI_src_PointCloud_PCregistCol_H_
#ifdef USE_OPEN3D
#include "../PCfilter/_PCtransform.h"
#include <open3d/pipelines/registration/ColoredICP.h>
using namespace open3d::pipelines::registration;

namespace kai
{

class _PCregistCol: public _ModuleBase
{
public:
	_PCregistCol();
	virtual ~_PCregistCol();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void console(void* pConsole);

private:
    void updateRegistration(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		(( _PCregistCol *) This)->update();
		return NULL;
	}

public:
	double m_rVoxel;
    double m_rNormal;
	int m_maxNNnormal;
    double m_rFitness;
    double m_rRMSE;
    int m_maxIter;

	_PCframe* m_pSrc;
	_PCframe* m_pTgt;
    RegistrationResult m_RR;
	_PCtransform* m_pTf;
    double m_lastFit;
};

}
#endif
#endif
