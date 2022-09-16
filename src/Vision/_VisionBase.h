/*
 * VisionBase.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__VisionBase_H_
#define OpenKAI_src_Vision__VisionBase_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"
#include "../Utility/utilCV.h"
#include "Frame.h"

namespace kai
{
	enum VISION_TYPE
	{
		vision_unknown,
		vision_camera,
		vision_gstreamer,
		vision_video,
		vision_realsense,
		vision_pylon,
		vision_raspivid,
		vision_file,
		vision_hiphen,
		vision_mynteye,
		vision_morphology,
		vision_threshold,
		vision_erode,
		vision_contrast,
		vision_histEqualize,
		vision_invert,
		vision_depth2Gray,
		vision_resize,
		vision_rotate,
		vision_inRange,
		vision_grayscale,
		vision_crop,
		vision_gphoto,
		vision_remap,
		vision_RSdepth,
		vision_DepthProj,
		vision_Vzense,
	};

	class _VisionBase : public _ModuleBase
	{
	public:
		_VisionBase();
		virtual ~_VisionBase();

		virtual bool init(void *pKiss);
		virtual void draw(void *pFrame);

		virtual bool open(void);
		virtual bool isOpened(void);
		virtual void close(void);

		virtual vInt2 getSize(void);
		virtual VISION_TYPE getType(void);
		virtual Frame *BGR(void);

	protected:
		bool m_bOpen;
		VISION_TYPE m_type;
		vInt2 m_vSize;

		Frame m_fBGR;
		vFloat4 m_bbDraw;
	};

}
#endif
