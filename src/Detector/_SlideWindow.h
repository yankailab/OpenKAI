/*
 * _SlideWindow.h
 *
 *  Created on: June 2, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__SlideWindow_H_
#define OpenKAI_src_Detector__SlideWindow_H_

#include "../Vision/_DepthVisionBase.h"

#ifdef USE_OPENCV
#include "_DNNclassifier.h"

namespace kai
{

class _SlideWindow : public _DetectorBase
{
public:
	_SlideWindow();
	virtual ~_SlideWindow();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void draw(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_SlideWindow*) This)->update();
		return NULL;
	}

public:
//	_DNNclassifier* m_pC;
	_VisionBase* m_pD;

	Mat		m_mD;
	Mat		m_mDin;
	Mat		m_mBGR;
	vFloat4 m_vRoi;
	float	m_w;
	float	m_dW;
	int		m_nW;
	float	m_maxD;
	float	m_dMinArea;
	vFloat2	m_dRange;
};

}
#endif
#endif
