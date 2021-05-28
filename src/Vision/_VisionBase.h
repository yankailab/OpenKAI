/*
 * VisionBase.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__VisionBase_H_
#define OpenKAI_src_Vision__VisionBase_H_
#ifdef USE_OPENCV
#include "../Base/_ModuleBase.h"
#include "../IO/_File.h"
#include "../UI/_Console.h"
#include "../UI/_WindowCV.h"

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
};

class _VisionBase: public _ModuleBase
{
public:
	_VisionBase();
	virtual ~_VisionBase();

	virtual bool init(void* pKiss);
	virtual void cvDraw(void* pWindow);

	virtual bool open(void);
	virtual bool isOpened(void);
	virtual void close(void);

	virtual vInt2 getSize(void);
	virtual VISION_TYPE getType(void);
	virtual Frame* BGR(void);

	bool setCamMatrices(const Mat& mC, const Mat& mD);
	bool scaleCamMatrices(void);
	vDouble2 getF(void);
	vDouble2 getC(void);
	Mat mC(void);
	Mat mCscaled(void);
	Mat mD(void);

protected:
	bool m_bOpen;
	VISION_TYPE m_type;
    vInt2 m_vSize;
    
	Frame	m_fBGR;
	vFloat4	m_bbDraw;

	//original camera matrix
	Mat m_mC;		//Intrinsic
	Mat m_mCscaled;	//scaled with input image size
	Mat m_mD;		//Distortion
	//Remap
	Mat m_m1;
	Mat m_m2;
};

}
#endif
#endif
