/*
 * _PCregistCol.h
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud_PCregistCol_H_
#define OpenKAI_src_3D_PointCloud_PCregistCol_H_

#include "../Pipeline/_PCtransform.h"
#include "../_PointCloud.h"
#include <open3d/pipelines/registration/ColoredICP.h>
using namespace open3d::pipelines::registration;

namespace kai
{

	class _PCregistCol : public _PointCloud
	{
	public:
		_PCregistCol();
		virtual ~_PCregistCol();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

		virtual void updatePC(void);

		double updateRegistration(PointCloud* pSrc, PointCloud* pTgt, Matrix4d_u* pTresult = NULL);

	private:
		bool updateRegistration(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_PCregistCol *)This)->update();
			return NULL;
		}

	protected:
		double m_maxDistance = 0.1;
		double m_rNormal = 0.2;
		int m_maxNNnormal = 30;
		double m_rFitness = 1e-6;
		double m_rRMSE = 1e-6;
		int m_maxIter = 30;

		// voxel down frame buf
		double m_rVoxel = 0.1;
		tSwap<PointCloud> m_sPCvd; //voxel down

		_PointCloud *m_pPC = nullptr;
		RegistrationResult m_RR;
		double m_minFit = 0.0;
		_PCtransform *m_pTf = nullptr;
	};

}
#endif
