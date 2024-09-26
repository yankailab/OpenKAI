/*
 * _MultiActuatorsBase.h
 *
 *  Created on: May 16, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__MultiActuatorsBase_H_
#define OpenKAI_src_Actuator__MultiActuatorsBase_H_

#include "_ActuatorBase.h"

#define AS_N_ACTUATOR 128

namespace kai
{

	class _MultiActuatorsBase : public _ModuleBase
	{
	public:
		_MultiActuatorsBase();
		~_MultiActuatorsBase();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);

		virtual void setPtarget(int iA, float nP);
		virtual void setStarget(int iA, float nS);
		virtual void gotoOrigin(int iA);
		virtual bool bComplete(int iA);

		virtual void setPtargetAll(float nP);
		virtual void setStargetAll(float nS);
		virtual void gotoOriginAll(void);
		virtual bool bCompleteAll(void);

	private:
		virtual void updateActuators(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_MultiActuatorsBase *)This)->update();
			return NULL;
		}

	public:
		_ActuatorBase *m_pAB[AS_N_ACTUATOR];
		int m_nAB;

		bool m_bPower;
		bool m_bReady;

	};

}
#endif
