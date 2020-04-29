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
	void draw(void);

	bool bReady(void);
	void reset(void);
	vFloat3 t(void);
	vFloat3 v(void);
	vFloat4 q(void);

protected:
	void resetAll(void);

public:
	bool	m_bReady;
	bool	m_bReset;
	vFloat3	m_vT;	//translation
	vFloat3	m_vV;	//velocity
	vFloat4 m_vQ;	//quaternion
	float	m_confidence;

};

}
#endif
