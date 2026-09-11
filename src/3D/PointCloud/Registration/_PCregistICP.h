/*
 * _PCregistICP.h
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud_PCregistICP_H_
#define OpenKAI_src_3D_PointCloud_PCregistICP_H_

#include "../Pipeline/_PCtransform.h"
using namespace open3d::pipelines::registration;

namespace kai
{

	enum PCREGIST_ICP_EST
	{
		icp_p2point = 0,
		icp_p2plane = 1,
	};

	class _PCregistICP : public _ModuleBase
	{
	public:
		_PCregistICP();
		virtual ~_PCregistICP();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	private:
		void updateRegistration(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_PCregistICP *)This)->update();
			return NULL;
		}

	protected:
		float m_thr = 0.02; // ICP threshold
		PCREGIST_ICP_EST m_est = icp_p2point;
		_PointCloud *m_pSrc = NULL;
		_PointCloud *m_pTgt = NULL;
		RegistrationResult m_RR;
		_PCtransform *m_pTf = NULL;
		double m_lastFit = 0.0;
	};

}
#endif
