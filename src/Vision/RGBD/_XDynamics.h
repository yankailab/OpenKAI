/*
 * _XDynamics.h
 *
 *  Created on: Jan 2, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RGBD__XDynamics_H_
#define OpenKAI_src_Vision_RGBD__XDynamics_H_

#include "_RGBDbase.h"
#include <xdyn_strm/xdyn_streamer.h>
#include <xdyn_strm/xdyn_define.h>
#include <xdyn_strm/xdyn_utils.h>
#include <xdyn_rgbd/sialib_TofRGBProc.h>
#include <xdyn_rgbd/siaGlobalOuter_RGBProc.h>

using namespace XD;

namespace kai
{
	struct XDctrl
	{
		vInt4 m_vPhaseInt;
		vInt4 m_vSpaceInt;
		vInt2 m_vFreq;

		int m_binning;
		int m_phaseMode;
		int m_mirrorMode;

		int m_rgbStride;
		int m_rgbFmt;

		bool m_bAutoExposureToF = true;
		int m_tExposureToF = 4000;

		bool m_bAutoExposureRGB = true;
		int m_tExposureRGB = 4000;

		bool m_bFilTime = false;
		int m_filTime = 0;

		bool m_bFilConfidence = true;
		int m_filConfidence = 1;

		bool m_bFilFlyingPix = false;
		int m_filFlyingPix = 0;

		bool m_bFillHole = false;
		bool m_bSpatialFilter = false;
		bool m_bHDR = false;

		void init(void)
		{
			m_vPhaseInt.set(1000000, 1000000, 0, 0);
			m_vSpaceInt.set(1000000, 1000000, 0, 0);
			m_vFreq.set(62, 25);

			m_binning = XDYN_BINNING_MODE_2x2;
			m_phaseMode = XDYN_PHASE_MODE_1;
			m_mirrorMode = XDYN_MIRROR_MODE_NONE;

			m_rgbStride = 0;
			m_rgbFmt = 4;
		}
	};

	struct XDdata
	{
		bool m_bInit;
		void *m_pD;

		CALIPARAS_RP m_RP;
		RP_DYNPARA m_dyn;
		RP_INPARAS m_in;
		RP_OUTPARAS m_out;

		void clear(void)
		{
			m_bInit = false;
			m_pD = NULL;
			memset(&m_RP, 0, sizeof(CALIPARAS_RP));
			memset(&m_dyn, 0, sizeof(RP_DYNPARA));
			memset(&m_in, 0, sizeof(RP_INPARAS));
			memset(&m_out, 0, sizeof(RP_OUTPARAS));
		}
	};

	class _XDynamics : public _RGBDbase
	{
	public:
		_XDynamics();
		virtual ~_XDynamics();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual int check(void);
		virtual bool start(void);
		virtual void console(void *pConsole);

		virtual bool open(void);
		virtual void close(void);

	protected:
		static void CbEvent(void *pHandle, int event, void *pData);
		static void CbStream(void *pHandle, MemSinkCfg *pCfg, XdynFrame_t *pData);
		void streamIn(MemSinkCfg *pCfg, XdynFrame_t *pData);
		bool initRGBD(XdynRegParams_t *pRegParams, uint16_t tofW, uint16_t tofH, uint16_t rgbW, uint16_t rgbH);
		void releaseRGBD(void);

		void updateXDynamics(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_XDynamics *)This)->update();
			return NULL;
		}

	private:
		int m_xdType;
		XdynCamInfo_t m_xdCamInfo;
		XDYN_Streamer *m_pXDstream;
		XDctrl m_xdCtrl;

		XDdata m_xdRGBD;

		Mat m_mXDyuv;
		Mat m_mXDd;
	};

}
#endif
