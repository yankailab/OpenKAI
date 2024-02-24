#ifndef OpenKAI_src_Autopilot_AP__AP_actuator_H_
#define OpenKAI_src_Autopilot_AP__AP_actuator_H_

#include "../../Actuator/_ActuatorBase.h"
#include "_AP_base.h"
#include "../../Utility/RC.h"

namespace kai
{

	class _AP_actuator : public _ModuleBase
	{
	public:
		_AP_actuator();
		~_AP_actuator();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		void update(void);
		void console(void *pConsole);

	private:
		void updateActuator(void);
		static void *getUpdate(void *This)
		{
			((_AP_actuator *)This)->update();
			return NULL;
		}

	private:
		_AP_base *m_pAP;
		_ActuatorBase *m_pAB1;
		_ActuatorBase *m_pAB2;

		RC_CHANNEL m_rcMode;
		RC_CHANNEL m_rcStickV;
		RC_CHANNEL m_rcStickH;
	};

}

#endif
