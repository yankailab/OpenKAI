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
	struct ARAREA_VERTEX
	{
		Vector3f m_vPw;
		vInt2 m_vPscr;
		
		void init(void)
		{
			m_vPw = {0,0,0};
			m_vPscr.init(0,0);
		}
	};

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
		bool world2Scr(const Vector3f &vPw, const Eigen::Affine3f &aA, vInt2* vPscr);
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
		vFloat2 m_vRange;

		// distance sensor offset in attitude sensor coordinate
		vFloat3 m_vDoriginA; // origin of the distance sensor (offset to attitude sensor)
		Vector3f m_vDptW;	// dynamic world coordinate of the point where the distance sensor is pointing at

		// camera offset in attitude sensor coordinate
		vFloat3 m_vCoriginA;
		Matrix4f m_mCoriginA;
		Eigen::Affine3f m_aW2C;
        vInt3 m_vAxisIdx;

		vector<ARAREA_VERTEX> m_vVertex;

	};

}
#endif
#endif
