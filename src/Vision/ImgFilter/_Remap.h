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

	void setCamMatrices(const Mat& mCam, const Mat& mDistCoeffs);
	Mat mC(void);
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
	Mat m_mCam;
	Mat m_mCamNew;
	Mat m_mDistCoeffs;
	Mat m_mX;
	Mat m_mY;
};

}
#endif
#endif
