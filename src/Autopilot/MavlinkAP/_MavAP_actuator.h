#ifndef OpenKAI_src_Autopilot_MavAP__MavAP_actuator_H_
#define OpenKAI_src_Autopilot_MavAP__MavAP_actuator_H_

#include "../../Actuator/_ActuatorBase.h"
#include "_MavAP_base.h"
#include "../../Utility/RC.h"

namespace kai
{

	class _MavAP_actuator : public _ModuleBase
	{
	public:
		_MavAP_actuator();
		~_MavAP_actuator();

		int init(void *pKiss);
		int start(void);
		int check(void);
		void update(void);
		void console(void *pConsole);

	private:
		void updateActuator(void);
		static void *getUpdate(void *This)
		{
			((_MavAP_actuator *)This)->update();
			return NULL;
		}

	private:
		_MavAP_base *m_pAP;
		_ActuatorBase *m_pAB1;
		_ActuatorBase *m_pAB2;

		RC_CHANNEL m_rcMode;
		RC_CHANNEL m_rcStickV;
		RC_CHANNEL m_rcStickH;
	};

}

#endif
