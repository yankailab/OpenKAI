/*
 * _PCregistGlobal.h
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud_PCregistGlobal_H_
#define OpenKAI_src_3D_PointCloud_PCregistGlobal_H_

#include "../PCfilter/_PCtransform.h"
#include <open3d/pipelines/registration/FastGlobalRegistration.h>
using namespace open3d::pipelines::registration;

namespace kai
{

	class _PCregistGlobal : public _ModuleBase
	{
	public:
		_PCregistGlobal();
		virtual ~_PCregistGlobal();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	private:
		std::shared_ptr<Feature> preprocess(PointCloud &pc);
		void updateRegistration(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_PCregistGlobal *)This)->update();
			return NULL;
		}

	protected:
		double m_rNormal;
		double m_rFeature;
		int m_maxNNnormal;
		int m_maxNNfpfh;

		_PCframe *m_pSrc;
		_PCframe *m_pTgt;
		RegistrationResult m_RR;
		_PCtransform *m_pTf;
		double m_lastFit;
	};

}
#endif
