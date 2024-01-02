/*
 * _XDynamics.h
 *
 *  Created on: Jan 2, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_LIDAR_XDynamics_H_
#define OpenKAI_src_LIDAR_XDynamics_H_

// #include "../3D/PointCloud/_PCframe.h"
#include "../Utility/util.h"
#include "../IPC/_SharedMem.h"

#include <xdyn_strm/xdyn_streamer.h>
#include <xdyn_strm/xdyn_define.h>
#include <xdyn_strm/xdyn_utils.h>
#include <xdyn_rgbd/sialib_TofRGBProc.h>
#include <xdyn_rgbd/siaGlobalOuter_RGBProc.h>

using namespace XD;

namespace kai
{
	struct UserHandler
	{
		XDYN_Streamer *stream;
		void *user;
		uint32_t imgSize;
		uint32_t other;

		// for rgbd cal
		bool isRgbdInit;
		void *rgbdHdl;
		CALIPARAS_RP rgbdRPRes;
		RP_DYNPARA rgbddynParas;
		RP_INPARAS rgbdInDatas;
		RP_OUTPARAS rgbdOutDatas;
	};

	class _XDynamics : public _ModuleBase
	{
	public:
		_XDynamics();
		virtual ~_XDynamics();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual bool open(void);
		virtual void close(void);

	protected:
	    int UserHdl_Init(UserHandler *usrHdl);
	    int UserHdl_InitRgbdHdl(UserHandler *usrHdl, XdynRegParams_t *regParams, uint16_t tofW, uint16_t tofH, uint16_t rgbW, uint16_t rgbH);
	    void UserHdl_RealseRgbdHdl(UserHandler *usrHdl);
	    void UserHdl_RgbdHdlImg(UserHandler *usrHdl);
	    void UserHdl_Release(UserHandler *usrHdl);

		static void CbEvent(void *handle, int event, void *data);
		static void CbStream(void *handle, MemSinkCfg *cfg, XdynFrame_t *data);

		void update(void);
		bool updateXDynamics(void);
		static void *getUpdate(void *This)
		{
			((_XDynamics *)This)->update();
			return NULL;
		}

		void updateTPP(void);
		static void *getTPP(void *This)
		{
			((_XDynamics *)This)->updateTPP();
			return NULL;
		}

	public:
		bool m_bOpen;
		vInt2 m_vSize;
		uint16_t m_tWait;

		bool m_bDepth;
		bool m_bIR;
		bool m_bRGB;
		bool m_btDepth;
		bool m_btRGB;
		float m_fConfidenceThreshold;

		// _SharedMem* m_psmRGB;
		// _SharedMem* m_psmDepth;
		// _SharedMem* m_psmTransformedDepth;
		// _SharedMem* m_psmTransformedRGB;
		// _SharedMem* m_psmIR;

		// depth filter processing thread
		_Thread *m_pTPP;
	};

}
#endif
