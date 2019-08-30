/*
 * VisionBase.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__VisionBase_H_
#define OpenKAI_src_Vision__VisionBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"

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
};

class _VisionBase: public _ThreadBase
{
public:
	_VisionBase();
	virtual ~_VisionBase();

	virtual bool init(void* pKiss);
	virtual bool draw(void);

	virtual bool open(void);
	virtual bool isOpened(void);
	virtual void close(void);

	virtual vInt2 getSize(void);
	virtual VISION_TYPE getType(void);
	virtual Frame* BGR(void);
	virtual void info(vInt2* pSize, vInt2* pCenter, vDouble2* pAngle);

public:
	bool m_bOpen;
	VISION_TYPE m_type;

	int m_w;
	int m_h;
	int m_cW;
	int m_cH;
	double m_fovW;
	double m_fovH;

	Frame	m_fBGR;
};

}
#endif
