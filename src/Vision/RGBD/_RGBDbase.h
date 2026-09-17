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
		virtual void draw(void *pMat);

		virtual int getData(void *pOut, int iD = 0, int nB = 0);

#ifdef USE_OPENCV
		virtual Mat *getDepthMat(void);
		virtual Vector2f getDepthRange(void);
#endif

	protected:
		// post processing thread
		_Thread *m_pTpp = nullptr;
		_IMUbase *m_pIMU = nullptr;

		int m_devFPSd = 30;
		Vector2i m_vSizeD = Vector2i(1280, 720);
		Vector2f m_vRangeD = Vector2f(0, FLT_MAX);
		float m_dScale = 1.0;	// calibration, only apply to CV_32UC1 m_mDepth
		float m_dOfs = 0.0;

		// switchs
		bool m_bDepth = true;
		bool m_bIR = false;
		bool m_btRGB = false;
		bool m_btDepth = false;
		bool m_bConfidence = true;
		float m_fConfidenceThr = 0.0;

		bool m_bIMU = false;
		bool m_bPCL = false;	// Depth point cloud
		bool m_bPCLrgb = false; // RGB point cloud

#ifdef USE_OPENCV
		Mat m_mDepth;
		Mat m_mtDepth;
		Mat m_mtRGB;
		Mat m_mIR;
#endif

#ifdef WITH_UNIVERSE
		_PointCloud *m_pPointCloud = nullptr;
#endif
	};

}
#endif
