/*
 * _GLIM.h
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__GLIM_H_
#define OpenKAI_src_SLAM__GLIM_H_

#include "_SLAMbase.h"
#include "../Universe/Geometry/PointCloud/_PointCloud.h"

namespace kai
{

	class _GLIM : public _SLAMbase
	{
	public:
		_GLIM();
		virtual ~_GLIM();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	private:
		virtual void updateGLIM(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_GLIM *)This)->update();
			return NULL;
		}

	protected:
		_PointCloud* m_pPCL = nullptr;

	};

}
#endif
