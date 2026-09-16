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

namespace kai
{

	class _NavBase : public _ModuleBase
	{
	public:
		_NavBase();
		virtual ~_NavBase();

		virtual bool init(const json& j);
		virtual void console(void *pConsole);

		virtual bool bOpen(void);
		virtual bool bError(void);

		virtual void reset(void);
		virtual float confidence(void);

		virtual Vector3f t(void);
		virtual Vector3f v(void);
		virtual Vector3f r(void);
		virtual Vector4f q(void);

		virtual const Eigen::Matrix4f &mT(void);
		virtual const Eigen::Matrix3f &mR(void);

	protected:
		virtual void resetAll(void);

	protected:
		BIT_FLAG m_flag;

		float m_scale = 1.0;
		Vector3i m_vAxisIdx = Vector3i::Zero(); //idx of yaw, pitch, roll
		Vector3f m_vT = Vector3f::Zero();	  //translation
		Vector3f m_vV = Vector3f::Zero();	  //velocity
		Vector3f m_vR = Vector3f::Zero();	  //rotation
		Vector4f m_vQ = Vector4f::Zero();	  //quaternion
		float m_confidence;

		Vector3f m_vRoffset = Vector3f::Zero(); //rotation offset

		Eigen::Matrix4f m_mT;
		Eigen::Matrix3f m_mR;
	};

}
#endif
