/*
 * _ARarea.h
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_Measurement__ARarea_H_
#define OpenKAI_src_Application_Measurement__ARarea_H_

#ifdef USE_OPENCV
#include "../../Vision/_VisionBase.h"
#include "../../SLAM/_SlamBase.h"
#include "../../Sensor/_DistSensorBase.h"

namespace kai
{

	class _ARarea : public _ModuleBase
	{
	public:
		_ARarea(void);
		virtual ~_ARarea();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		void cvDraw(void *pWindow);

	private:
		bool world2Scr(const Vector3d &vP, vInt2* vPscr);
		bool updateARarea(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ARarea *)This)->update();
			return NULL;
		}

	private:
		_VisionBase *m_pV;
		_DistSensorBase *m_pD;
		_SlamBase *m_pS;

		// distance sensor
		float m_d;
		vDouble2 m_vRange;

		// distance sensor in attitude sensor coordinate
		Vector3d m_vEyeOrigin;	// origin of the distance sensor (offset to attitude sensor)
		Vector3d m_vEye;	// dynamic world coordinate
		Vector3d m_vPtAt;	// dynamic world coordinate of the point where the distance sensor is pointing at

		// RGB offset in Lidar coordinate
		vDouble3 m_vToffsetRGB;
		vDouble3 m_vRoffsetRGB;
		Matrix4d m_mToffsetRGB;
		Eigen::Affine3d m_AoffsetRGB;
        vInt3 m_vAxisIdxRGB;
        vFloat3 m_vAxisKrgb;

	};

}
#endif
#endif
