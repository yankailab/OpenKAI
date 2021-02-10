/*
 * _Chilitags.h
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__Chilitags_H_
#define OpenKAI_src_Detector__Chilitags_H_

#include "../Detector/_DetectorBase.h"

#ifdef USE_OPENCV
#ifdef USE_CHILITAGS

#include <chilitags/chilitags.hpp>

using namespace chilitags;

namespace kai
{

class _Chilitags : public _DetectorBase
{
public:
	_Chilitags();
	virtual ~_Chilitags();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_Chilitags*) This)->update();
		return NULL;
	}

public:
    Chilitags m_chilitags;
    int m_persistence;
    float m_gain;
    float m_angleOffset;
};

}
#endif
#endif
#endif
