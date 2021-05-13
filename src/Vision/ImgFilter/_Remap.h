/*
 * _Remap.h
 *
 *  Created on: May 7, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Remap_H_
#define OpenKAI_src_Vision__Remap_H_

#ifdef USE_OPENCV
#include "../_VisionBase.h"

namespace kai
{

class _Remap: public _VisionBase
{
public:
	_Remap();
	virtual ~_Remap();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);
	bool bReady(void);

	bool setup(void);
	bool setCamMatrices(const Mat& mC, const Mat& mD);
	Mat mC(void);
	Mat mCscaled(void);
	Mat mD(void);

private:
	void filter(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_Remap*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pV;
	string m_yml;
	bool m_bReady;

	//original
	Mat m_mC;
	Mat m_mD;

	//scaled with input image size
	Mat m_mCscaled;
	Mat m_m1;
	Mat m_m2;
	cv::Size m_s;
};

}
#endif
#endif
