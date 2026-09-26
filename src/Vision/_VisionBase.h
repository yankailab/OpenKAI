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
#include "../Protocol/_JSONbase.h"

#ifdef USE_OPENCV
#include <mutex>
#include "../Utility/utilCV.h"
#include "../Base/cv.h"
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
		vision_remap,
		vision_RSdepth,
		vision_SharedMemImg,
		vision_BGR2HSV,
		vision_depth2Gray,
		vision_D2RGB,
		vision_thermal2RGB,
	};

	class _VisionBase : public _ModuleBase
	{
	public:
		_VisionBase();
		virtual ~_VisionBase();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual bool check(void);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);
		virtual void draw(void *pMat);

		virtual bool open(void);
		virtual bool bOpened(void);
		virtual void close(void);

		virtual VISION_TYPE getType(void);
		virtual Vector2i getSizeRGB(void);

#ifdef USE_OPENCV
		// Raw views require capture to be stopped; use copies for concurrent readers.
		virtual Mat *getMatRGB(void);
		virtual void copyMatRGB(Mat &m);
#endif

	protected:
		VISION_TYPE m_type = vision_unknown;

		string m_devURI = "";
		bool m_bRGB = true;
		int m_devFPS = 30; // device native FPS
		Vector2i m_vSizeRGB = Vector2i(1280, 720);

		bool m_bOpened = false;

#ifdef USE_OPENCV
		// Hold while reading or writing m_mRGB, including its pixels.
		std::mutex m_mutexRGB;
		Mat m_mRGB;	// CV_8UC3
#endif
	};

}
#endif
