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
		static void CbEvent(void *handle, int event, void *data);
		static void CbStream(void *handle, MemSinkCfg *cfg, XdynFrame_t *data);
		void streamIn(MemSinkCfg *cfg, XdynFrame_t *data);
		bool initRGBD(XdynRegParams_t *regParams, uint16_t tofW, uint16_t tofH, uint16_t rgbW, uint16_t rgbH);
		void releaseRGBD(void);

		bool updateXDynamics(void);
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
		XDctrl m_XDctrl;

		XDdata m_xdRGBD;
	};

}
#endif
