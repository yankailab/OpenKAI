/*
 * _RGBDpointCloudBase.h
 *
 *  Created on: Jan 8, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RGBD__RGBDpointCloudBase_H_
#define OpenKAI_src_Vision_RGBD__RGBDpointCloudBase_H_

#include "_RGBDbase.h"
#ifdef USE_OPEN3D
#include "../../3D/PointCloud/_PCframe.h"
#endif

namespace kai
{
	class _RGBDpointCloudBase : public _RGBDbase
	{
	public:
		_RGBDpointCloudBase();
		virtual ~_RGBDpointCloudBase();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:

	};

}
#endif
