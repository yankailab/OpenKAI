/*
 * _SlamBase.h
 *
 *  Created on: May 29, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__SlamBase_H_
#define OpenKAI_src_SLAM__SlamBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"

namespace kai
{

class _SlamBase: public _ThreadBase
{
public:
	_SlamBase();
	virtual ~_SlamBase();

	bool init(void* pKiss);
	void detect(void);
	void draw(void);

	void reset(void);

public:
	bool	 m_bReady;
	vDouble3 m_pos;

};

}
#endif
