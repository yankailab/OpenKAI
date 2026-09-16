/*
 * _RGBDbase.h
 *
 *  Created on: Jan 2, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RGBD__RGBDbase_H_
#define OpenKAI_src_Vision_RGBD__RGBDbase_H_

#include "../_VisionBase.h"
#include "../../Sensor/_IMUbase.h"

#ifdef USE_OPENCV
#include "../../Utility/utilCV.h"
#include "../../Vision/Frame.h"
#endif

#ifdef WITH_UNIVERSE
#include "../../Universe/Geometry/PointCloud/_PointCloud.h"
#endif

namespace kai
{
	class _RGBDbase : public _VisionBase
	{
	public:
		_RGBDbase();
		virtual ~_RGBDbase();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool check(void);
		virtual void console(void *pConsole);
		virtual void draw(void *pFrame);

		virtual bool open(void);
		virtual void close(void);

#ifdef USE_OPENCV
		virtual Frame *getFrameD(void);
		virtual Vector2f getRangeD(void);
		virtual float d(const Vector4i &bb);
		virtual float d(const Vector4f &bb);
#endif

	protected:
		// post processing thread
		_Thread *m_pTpp = nullptr;
		_IMUbase* m_pIMU = nullptr;

		int m_devFPSd = 30;
		Vector2i m_vSizeD = Vector2i::Zero();
		Vector2f m_vRangeD = Vector2f::Zero();

		// switchs
		bool m_bDepth = true;
		bool m_bIR = false;
		bool m_btRGB = false;
		bool m_btDepth = false;
		bool m_bConfidence = true;
		float m_fConfidenceThreshold = 0.0;

		bool m_bIMU = false;
		bool m_bPCd = false;   // Depth point cloud
		bool m_bPCrgb = false; // RGB point cloud

#ifdef USE_OPENCV
		Frame m_fDepth;
		Frame m_fTfDepth;
		Frame m_fTfRGB;
		Frame m_fIR;

		float m_dScale = 1.0;
		float m_dOfs = 0.0;
		int m_nHistLev = 128;
		int m_iHistFrom = 0;
		float m_minHistD = 0.25;

		bool m_bDebugDepth = 0;
#endif

#ifdef WITH_UNIVERSE
		_PointCloud *m_pPointCloud = nullptr;
#endif
	};

}
#endif
