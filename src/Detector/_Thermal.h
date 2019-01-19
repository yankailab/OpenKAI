/*
 * _Thermal.h
 *
 *  Created on: Jan 18, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__Thermal_H_
#define OpenKAI_src_Detector__Thermal_H_

#include "../Base/common.h"
#include "../Base/_ObjectBase.h"


namespace kai
{

class _Thermal : public _ObjectBase
{
public:
	_Thermal();
	virtual ~_Thermal();

	bool init(void* pKiss);
	bool start(void);
	int check(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Thermal*) This)->update();
		return NULL;
	}

public:
	double	m_abs;
	double	m_scale1;
	double	m_scale2;
	double	m_thr1;
	double	m_thr2;

#ifdef USE_CUDA

#endif

};
}

#endif
