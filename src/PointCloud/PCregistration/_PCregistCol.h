/*
 * _PCregistCol.h
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCregistCol_H_
#define OpenKAI_src_PointCloud_PCregistCol_H_
#ifdef USE_OPEN3D

#include "../_PCbase.h"
#include <open3d/pipelines/registration/ColoredICP.h>

using namespace open3d::pipelines::registration;

namespace kai
{

class _PCregistCol: public _PCbase
{
public:
	_PCregistCol();
	virtual ~_PCregistCol();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void draw(void);

private:
    bool ICPcolorRegistration(void);
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

	_PCbase* m_pSrc;
	_PCbase* m_pTgt;
	int m_iMt;
    RegistrationResult m_RR;
    double m_lastFit;
};

}
#endif
#endif
