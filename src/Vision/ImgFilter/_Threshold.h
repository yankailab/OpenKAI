/*
 * _Threshold.h
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Threshold_H_
#define OpenKAI_src_Vision__Threshold_H_

#include "../../Base/common.h"

#ifdef USE_OPENCV
#include "../_VisionBase.h"

namespace kai
{

enum IMG_THR_TYPE
{
	img_thr,
	img_thr_adaptive,
};

struct IMG_THRESHOLD
{
	int		m_type;
	double	m_vMax;
	int		m_method;
	bool	m_bAutoThr;
	double	m_thr;
	int		m_thrType;
	int		m_blockSize;
	double	m_C;

	void init(void)
	{
		m_type = img_thr_adaptive;
		m_vMax = 1.0;
		m_bAutoThr = false;
		m_thr = 0.0;
		m_method = ADAPTIVE_THRESH_GAUSSIAN_C;//ADAPTIVE_THRESH_MEDIAN_C
		m_thrType = THRESH_BINARY;//THRESH_BINARY_INV
		m_blockSize = 3;
		m_C = 0.0;
	}
};

class _Threshold: public _VisionBase
{
public:
	_Threshold();
	virtual ~_Threshold();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);

private:
	void filter(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_Threshold*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pV;
	Frame m_fIn;
	vector<IMG_THRESHOLD> m_vFilter;

};

}
#endif
#endif
