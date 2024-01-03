/*
 * _RGBDbase.h
 *
 *  Created on: Jan 2, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_RGBD_RGBDbase_H_
#define OpenKAI_src_RGBD_RGBDbase_H_

// #include "../3D/PointCloud/_PCframe.h"
#include "../../Utility/util.h"
#include "../../IPC/_SharedMem.h"
#include "../../UI/_Console.h"

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

		virtual bool open(void);
		virtual void close(void);

	protected:
		string m_deviceURI;
		bool m_bOpen;
		vInt2 m_vSize;
		uint16_t m_tWait;

		bool m_bDepth;
		bool m_bIR;
		bool m_bRGB;
		bool m_btDepth;
		bool m_btRGB;
		float m_fConfidenceThreshold;

		_SharedMem* m_psmRGB;
		_SharedMem* m_psmDepth;
		_SharedMem* m_psmTransformedDepth;
		_SharedMem* m_psmTransformedRGB;
		_SharedMem* m_psmIR;

		// post processing thread
		_Thread *m_pTPP;
	};

}
#endif
