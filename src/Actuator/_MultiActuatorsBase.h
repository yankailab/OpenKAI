/*
 * _MultiActuatorsBase.h
 *
 *  Created on: May 16, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__MultiActuatorsBase_H_
#define OpenKAI_src_Actuator__MultiActuatorsBase_H_

#include "_ActuatorBase.h"

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

		virtual void setPtargetAll(float nP);
		virtual void setStargetAll(float nS);
		virtual void gotoOriginAll(void);
		virtual bool bCompleteAll(void);

	protected:
		virtual void updateActuators(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_MultiActuatorsBase *)This)->update();
			return NULL;
		}

	protected:
		vector<_ActuatorBase*> m_vAB;
	};

}
#endif
