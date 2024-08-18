#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_actuator_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_actuator_H_

#include "../../Actuator/_ActuatorBase.h"
#include "_APmavlink_base.h"
#include "../../Utility/RC.h"

namespace kai
{

	class _APmavlink_actuator : public _ModuleBase
	{
	public:
		_APmavlink_actuator();
		~_APmavlink_actuator();

		int init(void *pKiss);
		int start(void);
		int check(void);
		void update(void);
		void console(void *pConsole);

	private:
		void updateActuator(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_actuator *)This)->update();
			return NULL;
		}

	private:
		_APmavlink_base *m_pAP;
		_ActuatorBase *m_pAB1;
		_ActuatorBase *m_pAB2;

		RC_CHANNEL m_rcMode;
		RC_CHANNEL m_rcStickV;
		RC_CHANNEL m_rcStickH;
	};

}

#endif
