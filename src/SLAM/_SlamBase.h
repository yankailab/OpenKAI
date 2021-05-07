/*
 * _SlamBase.h
 *
 *  Created on: May 29, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__SlamBase_H_
#define OpenKAI_src_SLAM__SlamBase_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"
using namespace Eigen;

namespace kai
{

class _SlamBase: public _ModuleBase
{
public:
	_SlamBase();
	virtual ~_SlamBase();

	virtual bool init(void* pKiss);
	virtual void console(void* pConsole);

	virtual bool bReady(void);
	virtual void reset(void);
	virtual vFloat3 t(void);
	virtual vFloat3 v(void);
	virtual vFloat3 r(void);
	virtual vFloat4 q(void);
	virtual const Matrix4d& mT(void);

protected:
	virtual void resetAll(void);

protected:
	bool	m_bReady;
	bool	m_bReset;

	vInt3 m_vAxisIdx; //idx of yaw, pitch, roll
	vFloat3	m_vT;	//translation
	vFloat3	m_vV;	//velocity
	vFloat3 m_vR;	//rotation
	vFloat4 m_vQ;	//quaternion
	float	m_confidence;

	vFloat3 m_vRoffset;	//rotation offset

	Matrix4d m_mT;

};

}
#endif
