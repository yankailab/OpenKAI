/*
 * _RGBDbase.h
 *
 *  Created on: Jan 2, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_RGBD_RGBDbase_H_
#define OpenKAI_src_RGBD_RGBDbase_H_

// #include "../3D/PointCloud/_PCframe.h"
//#include "../../Utility/util.h"
#include "../../IPC/_SharedMem.h"
#include "../../UI/_Console.h"

#ifdef USE_OPENCV
#include "../../Utility/utilCV.h"
#include "../../Vision/Frame.h"
#endif

namespace kai
{
	class _RGBDbase : public _ModuleBase
	{
	public:
		_RGBDbase();
		virtual ~_RGBDbase();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual int check(void);
		virtual void console(void *pConsole);
		virtual void draw(void *pFrame);

		virtual bool open(void);
		virtual void close(void);

#ifdef USE_OPENCV
		virtual Frame *RGB(void);
		virtual Frame *Depth(void);
		virtual float d(const vInt4 &bb);
		virtual float d(const vFloat4 &bb);
#endif

	protected:
		// post processing thread
		_Thread *m_pTPP;

		string m_deviceURI;
		bool m_bOpen;
		vInt2 m_vSizeRGB;
		vInt2 m_vSizeD;
		vFloat2 m_vRz; //z region crop
		uint16_t m_tFrameInterval;	// minimal interval between frame reading

		bool m_bRGB;
		bool m_bDepth;
		bool m_bIR;
		bool m_btRGB;
		bool m_btDepth;
		float m_fConfidenceThreshold;

		// frames
		_SharedMem *m_psmRGB;
		_SharedMem *m_psmDepth;
		_SharedMem *m_psmTransformedDepth;
		_SharedMem *m_psmTransformedRGB;
		_SharedMem *m_psmIR;
		
#ifdef USE_OPENCV
		Frame m_fRGB;
		Frame m_fDepth;
		vFloat2 m_vRange;
		float m_dScale;
		float m_dOfs;
		int m_nHistLev;
		int m_iHistFrom;
		float m_minHistD;
#endif

	};

}
#endif
