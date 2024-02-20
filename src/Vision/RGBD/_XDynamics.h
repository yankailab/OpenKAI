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
		vInt4 m_vPhaseInt = {1000000, 1000000, 0, 0};
		vInt4 m_vSpaceInt = {1000000, 1000000, 0, 0};
		vInt2 m_vFreq = {62, 25};

		int m_binning = XDYN_BINNING_MODE_2x2;
		int m_phaseMode = XDYN_PHASE_MODE_1;
		int m_mirrorMode = XDYN_MIRROR_MODE_NONE;
		int m_algMode = XDYN_ALG_MODE_EMB_ALG_IPC_PASS;

		int m_rgbStride = 0;
		int m_rgbFmt = 4;

		uint8_t m_bAE = 1;
		uint16_t m_preDist = 4000;

		uint8_t m_DtdnMethod = 0;
		uint8_t m_DtdnLev = 0;
		uint8_t m_DsdnMethod = 0;
		uint8_t m_DsdnLev = 0;

		uint8_t m_GtdnMethod = 0;
		uint8_t m_GtdnLev = 0;
		uint8_t m_GsdnMethod = 0;
		uint8_t m_GsdnLev = 0;

		uint8_t m_dFlyPixLev = 0;
	};

	struct XDhdl
	{
		bool m_bInit;
		void *m_pHDL;

		CALIPARAS_RP m_RP;
		RP_DYNPARA m_dyn;
		RP_INPARAS m_in;
		RP_OUTPARAS m_out;

		void init(void)
		{
			m_bInit = false;
			m_pHDL = NULL;
			memset(&m_RP, 0, sizeof(CALIPARAS_RP));
			memset(&m_dyn, 0, sizeof(RP_DYNPARA));
			memset(&m_in, 0, sizeof(RP_INPARAS));
			memset(&m_out, 0, sizeof(RP_OUTPARAS));
		}

		void release(void)
		{
			IF_(!m_bInit);
			NULL_(m_pHDL);

			sitrpRelease(&m_pHDL, FALSE);
			m_pHDL = NULL;
			m_bInit = false;

			if(m_out.pstrRGBD)
				free(m_out.pstrRGBD);
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

		static void sCbEvent(void *pHandle, int event, void *pData)
		{
			_XDynamics *pXD = (_XDynamics *)pHandle;

			if (event == XDYN_CB_EVENT_DEVICE_DISCONNECT)
			{
				LOG(INFO) << *pXD->getName() << ": "
						  << "Device disconnected";
				pXD->close();
			}
		}

		static void sCbStream(void *pHandle, MemSinkCfg *pCfg, XdynFrame_t *pData)
		{
			NULL_(pHandle);
			_XDynamics *pXD = (_XDynamics *)pHandle;
			pXD->cbStream(pCfg, pData);
		}

	protected:
		void cbStream(MemSinkCfg *pCfg, XdynFrame_t *pData);
		void runHDL(
			unsigned short* pD,
			unsigned char* pRGB,
			unsigned char* pC
		);
		bool initHDL(XdynRegParams_t *pRegParams, uint16_t tofW, uint16_t tofH, uint16_t rgbW, uint16_t rgbH);
		void releaseHDL(void);

		void updateXDynamics(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_XDynamics *)This)->update();
			return NULL;
		}

	private:
		int m_xdDevType;
		int m_xdProductType;
		XdynCamInfo_t m_xdCamInfo;
		XDYN_Streamer *m_pXDstream;
		XDctrl m_xdCtrl;
		XDhdl m_xdHDL;

		Mat m_mXDyuv;
		Mat m_mXDd;
	};

}
#endif
