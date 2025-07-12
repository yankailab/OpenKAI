/*
 * _DepthProj.h
 *
 *  Created on: June 15, 2022
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__DepthProj_H_
#define OpenKAI_src_Vision__DepthProj_H_

#include "../RGBD/_RGBDbase.h"

namespace kai
{

	class _DepthProj : public _VisionBase
	{
	public:
		_DepthProj();
		virtual ~_DepthProj();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);

		void createFilterMat(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_DepthProj *)This)->update();
			return NULL;
		}

	protected:
		_RGBDbase *m_pV;
		Mat m_mF;

		float m_fFov;
		vFloat2 m_vCenter;
	};

}
#endif
