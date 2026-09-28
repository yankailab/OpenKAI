/*
 * _RGBDbase.h
 *
 *  Created on: Jan 2, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RGBD__RGBDbase_H_
#define OpenKAI_src_Vision_RGBD__RGBDbase_H_

#include "../../DataStream/RGBDframe.h"
#include "../../DataStream/PCLframe.h"
#include "../../DataStream/IMUstream.h"

#include "../_RGBbase.h"

namespace kai
{
	class _RGBDbase : public _RGBbase
	{
	public:
		_RGBDbase();
		virtual ~_RGBDbase();

		virtual bool loadConfig(void) override;
		virtual bool saveConfig(bool bExport) override;

		virtual bool link(InstanceMgr *pM) override;
		virtual bool check(void);
		virtual void console(void *pConsole);

		virtual Vector2f getDepthRange(void);
		virtual float getDepthScale(void);
		virtual float getDepthOffset(void);

	protected:
		// Output frames own their pixels; depth channels are calibrated CV_32FC1.
		RGBDframe *m_pRGBD = nullptr;
		RGBDframe *m_pRGBDtRGB = nullptr;	// RGBD transformed to RGB
		RGBDframe *m_pRGBDtD = nullptr;		// RGBD transformed to D

		RGBframe *m_pD = nullptr;			// real distance unit float mat
		RGBframe *m_pIR = nullptr;

		PCLframe *m_pPCL = nullptr;
		IMUstream *m_pIMU = nullptr;

		// post processing thread
		_Thread *m_pTpp = nullptr;

		// config
		int m_devFPSd = 30;
		Vector2i m_vSizeD = Vector2i(1280, 720);
		Vector2f m_vRangeD = Vector2f(0, FLT_MAX);
		float m_dScale = 1.0; // scaling calibration
		float m_dOfs = 0.0;	  // offset calibration

		// switches
		bool m_bDepth = true;
		bool m_btDepth = false;
		bool m_btRGB = false;
		bool m_bIR = false;
		bool m_bConfidence = true;
		float m_fConfidenceThr = 0.0;

		bool m_bIMU = false;
		bool m_bPCL = false;	// Depth point cloud
		bool m_bPCLrgb = false; // RGB point cloud
	};

}
#endif
