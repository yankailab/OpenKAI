/*
 * VisionBase.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__VisionBase_H_
#define OpenKAI_src_Vision__VisionBase_H_

#include "../IPC/SharedMem.h"
#include "../UI/_Console.h"

#ifdef USE_OPENCV
#include "../Utility/utilCV.h"
#include "Frame.h"
#endif

namespace kai
{
	enum VISION_TYPE
	{
		vision_unknown,
		vision_camera,
		vision_gstreamer,
		vision_video,
		vision_realsense,
		vision_orbbec,
		vision_uvc,
		vision_file,
		vision_morphology,
		vision_threshold,
		vision_erode,
		vision_contrast,
		vision_histEqualize,
		vision_invert,
		vision_resize,
		vision_rotate,
		vision_inRange,
		vision_colorConvert,
		vision_crop,
		vision_gphoto,
		vision_remap,
		vision_RSdepth,
		vision_SharedMemImg,
		vision_BGR2HSV,
		vision_depth2Gray,
		vision_D2RGB
	};

	class _VisionBase : public _ModuleBase
	{
	public:
		_VisionBase();
		virtual ~_VisionBase();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int check(void);
		virtual void console(void *pConsole);
		virtual void draw(void *pFrame);

		virtual bool open(void);
		virtual bool isOpened(void);
		virtual void close(void);

		virtual vInt2 getSize(void);
		virtual VISION_TYPE getType(void);

#ifdef USE_OPENCV
		virtual Frame *getFrameRGB(void);
#endif

	protected:
		VISION_TYPE m_type;
		string m_devURI;
		int m_devFPS;	// device native FPS
		uint64_t m_tFrameInterval;	// minimal interval between frame reading
		bool m_bRGB;
		vInt2 m_vSizeRGB;

		bool m_bOpen;

		SharedMem *m_psmRGB;

#ifdef USE_OPENCV
		Frame m_fRGB;
#endif
	};

}
#endif
