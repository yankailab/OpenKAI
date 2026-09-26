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

		virtual bool loadConfig(void) override;
		virtual bool link(void) override;
		virtual bool check(void);
		virtual void console(void *pConsole);
		virtual void draw(void *pMat);

		virtual Vector2f getDepthRange(void);
		virtual float getDepthScale(void);
		virtual float getDepthOffset(void);

#ifdef USE_OPENCV
		// Raw views require capture to be stopped; use copies for concurrent readers.
		virtual Mat *getMatDepth(void);
		virtual void copyMatDepth(Mat &m);
		virtual Mat *getMatTransformedDepth(void);
		virtual void copyMatTransformedDepth(Mat &m);
		virtual Mat *getMatTransformedRGB(void);
		virtual void copyMatTransformedRGB(Mat &m);
		virtual Mat *getMatIR(void);
		virtual void copyMatIR(Mat &m);
#endif

	protected:
		// post processing thread
		_Thread *m_pTpp = nullptr;
		_IMUbase *m_pIMU = nullptr;

		int m_devFPSd = 30;
		Vector2i m_vSizeD = Vector2i(1280, 720);
		Vector2f m_vRangeD = Vector2f(0, FLT_MAX);
		float m_dScale = 1.0; // scaling calibration
		float m_dOfs = 0.0;	  // offset calibration

		// switchs
		bool m_bDepth = true;
		bool m_btDepth = false;
		bool m_btRGB = false;
		bool m_bIR = false;
		bool m_bConfidence = true;
		float m_fConfidenceThr = 0.0;

		bool m_bIMU = false;
		bool m_bPCL = false;	// Depth point cloud
		bool m_bPCLrgb = false; // RGB point cloud

#ifdef USE_OPENCV
		// Guards depth, transformed depth and IR; m_mutexRGB also guards m_mtRGB.
		std::mutex m_mutexDepth;
		Mat m_mDepth;  // device native format, usually CV_16UC1
		Mat m_mtDepth; // the same as mDepth
		Mat m_mtRGB;   // CV_8UC3
		Mat m_mIR;	   // CV_8UC1
#endif

#ifdef WITH_UNIVERSE
		_PointCloud *m_pPCL = nullptr;
#endif
	};

}
#endif
