/*
 * _ActuatorSync.h
 *
 *  Created on: May 16, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__ActuatorSync_H_
#define OpenKAI_src_Actuator__ActuatorSync_H_

#include "_ActuatorBase.h"

#define AS_N_ACTUATOR 128

namespace kai
{

	class _ActuatorSync : public _ActuatorBase
	{
	public:
		_ActuatorSync();
		~_ActuatorSync();

		virtual int init(void *pKiss);
		virtual int start(void);
		virtual int check(void);

		virtual void setPtarget(int i, float nP);
		virtual void setStarget(int i, float nS);
		virtual void gotoOrigin(void);
		virtual bool bComplete(void);

		virtual float getP(int i);
		virtual float getS(int i);

	private:
		virtual void updateSync(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_ActuatorSync *)This)->update();
			return NULL;
		}

	public:
		_ActuatorBase *m_pAB[AS_N_ACTUATOR];
		int m_nAB;
		int m_iABget;
	};

}
#endif
