/*
 * _XDynamics.h
 *
 *  Created on: Jan 2, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_RGBD_XDynamics_H_
#define OpenKAI_src_RGBD_XDynamics_H_

#include "_RGBDbase.h"
#include <xdyn_strm/xdyn_streamer.h>
#include <xdyn_strm/xdyn_define.h>
#include <xdyn_strm/xdyn_utils.h>
#include <xdyn_rgbd/sialib_TofRGBProc.h>
#include <xdyn_rgbd/siaGlobalOuter_RGBProc.h>

using namespace XD;

namespace kai
{
	class _XDynamics : public _RGBDbase
	{
	public:
		_XDynamics();
		virtual ~_XDynamics();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
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
		XDYN_Streamer *m_pXDstream;
		bool m_bXDrgbdInit;
		void *m_pXDrgbd;
		CALIPARAS_RP m_XDrgbdRPRes;
		RP_DYNPARA m_XDrgbddynParas;
		RP_INPARAS m_XDrgbdInData;
		RP_OUTPARAS m_XDrgbdOutData;
	};

}
#endif
