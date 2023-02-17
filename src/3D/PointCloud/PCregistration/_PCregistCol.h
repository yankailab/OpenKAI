/*
 * _PCregistCol.h
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud_PCregistCol_H_
#define OpenKAI_src_3D_PointCloud_PCregistCol_H_

#include "../PCfilter/_PCtransform.h"
#include "../_PCframe.h"
#include <open3d/pipelines/registration/ColoredICP.h>
using namespace open3d::pipelines::registration;

namespace kai
{

	class _PCregistCol : public _PCframe
	{
	public:
		_PCregistCol();
		virtual ~_PCregistCol();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		virtual void updatePC(void);

		double updateRegistration(PointCloud* pSrc, PointCloud* pTgt, Matrix4d_u* pTresult = NULL);

	private:
		bool updateRegistration(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_PCregistCol *)This)->update();
			return NULL;
		}

	public:
		double m_maxDistance;
		double m_rNormal;
		int m_maxNNnormal;
		double m_rFitness;
		double m_rRMSE;
		int m_maxIter;

		// voxel down frame buf
		double m_rVoxel;
		tSwap<PointCloud> m_sPCvd; //voxel down

		_PCframe *m_pPCf;
		RegistrationResult m_RR;
		double m_minFit;
		_PCtransform *m_pTf;
	};

}
#endif
