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
#include "../../Navigation/_NavBase.h"
#include "../../Sensor/_DistSensorBase.h"

namespace kai
{
	struct ARAREA_VERTEX
	{
		Vector3f m_vVertW;	//world coordinate
		cv::Point m_vPs;	//screen
		bool m_bP;			//if inside the projection plane
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

		// UI handler
		static void sOnBtnAdd(void* pInst);
		static void sOnBtnClear(void* pInst);
		static void sOnBtnSave(void* pInst);

		void addVertex(void);

	protected:
		bool updateARarea(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ARarea *)This)->update();
			return NULL;
		}

		bool c2scr(const Vector3f &vPc,
				 const cv::Size& vSize,
				 const vFloat2& vF,
				 const vFloat2& vC,				 
				 cv::Point* pvPs);
		float area(void);

	private:
		_VisionBase *m_pV;
		_DistSensorBase *m_pD;
		_NavBase *m_pN;

		// pose
		Eigen::Affine3f m_aPose;
		Vector3f m_vDptP;

		// distance sensor
		float m_d;
		vFloat2 m_vRange;
		vFloat3 m_vDorgP;	// distance sensor offset in pose sensor coordinate
		Vector3f m_vDptW;	// point where the distance sensor is pointing at in world coordinate

		// camera
		vFloat3 m_vCorgP;	// camera offset in pose sensor coordinate
		Eigen::Affine3f m_aW2C;
        vInt3 m_vAxisIdx;

		// area
		vector<ARAREA_VERTEX> m_vVert;

	};

}
#endif
#endif
