/*
 * _NavBase.h
 *
 *  Created on: May 29, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Navigation__NavBase_H_
#define OpenKAI_src_Navigation__NavBase_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"
#include "../Utility/BitFlag.h"
using namespace Eigen;

namespace kai
{

	class _NavBase : public _ModuleBase
	{
	public:
		_NavBase();
		virtual ~_NavBase();

		virtual bool init(void *pKiss);
		virtual void console(void *pConsole);

		virtual bool bOpen(void);
		virtual bool bError(void);

		virtual void reset(void);
		virtual float confidence(void);

		virtual vFloat3 t(void);
		virtual vFloat3 v(void);
		virtual vFloat3 r(void);
		virtual vFloat4 q(void);

		virtual const Matrix4f &mT(void);
		virtual const Matrix3f &mR(void);

	protected:
		virtual void resetAll(void);

	protected:
		BIT_FLAG m_flag;

		float m_scale;
		vInt3 m_vAxisIdx; //idx of yaw, pitch, roll
		vFloat3 m_vT;	  //translation
		vFloat3 m_vV;	  //velocity
		vFloat3 m_vR;	  //rotation
		vFloat4 m_vQ;	  //quaternion
		float m_confidence;

		vFloat3 m_vRoffset; //rotation offset

		Matrix4f m_mT;
		Matrix3f m_mR;
	};

}
#endif
