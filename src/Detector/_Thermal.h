/*
 * _Thermal.h
 *
 *  Created on: Jan 18, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__Thermal_H_
#define OpenKAI_src_Detector__Thermal_H_

#include "_DetectorBase.h"

#ifdef USE_OPENCV

namespace kai
{

class _Thermal : public _DetectorBase
{
public:
	_Thermal();
	virtual ~_Thermal();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void draw(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Thermal*) This)->update();
		return NULL;
	}

public:
	Mat m_mR;
	double	m_rL;
	double	m_rU;

};

}
#endif
#endif
