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

#ifdef WITH_3D
#include "../../3D/PointCloud/_PCstream.h"
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
		virtual vFloat2 getRangeD(void);
		virtual float d(const vInt4 &bb);
		virtual float d(const vFloat4 &bb);
#endif

	protected:
		// post processing thread
		_Thread *m_pTpp;
		_IMUbase* m_pIMU;

		int m_devFPSd;
		vInt2 m_vSizeD;
		vFloat2 m_vRangeD;

		// switchs
		bool m_bDepth;
		bool m_bIR;
		bool m_btRGB;
		bool m_btDepth;
		bool m_bConfidence;
		float m_fConfidenceThreshold;

		bool m_bIMU;
		bool m_bPCd;   // Depth point cloud
		bool m_bPCrgb; // RGB point cloud

#ifdef USE_OPENCV
		Frame m_fDepth;
		Frame m_fTfDepth;
		Frame m_fTfRGB;
		Frame m_fIR;

		float m_dScale;
		float m_dOfs;
		int m_nHistLev;
		int m_iHistFrom;
		float m_minHistD;

		bool m_bDebugDepth;
#endif

#ifdef WITH_3D
		_PCstream *m_pPCstream;
#endif
	};

}
#endif
