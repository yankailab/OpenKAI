#ifndef OpenKAI_src_Application_Rover__AProver_WB_H_
#define OpenKAI_src_Application_Rover__AProver_WB_H_

#include "../../Autopilot/ArduPilot/_AP_base.h"
#include "../../Autopilot/ArduPilot/_AProver_drive.h"
#include "../../Universe/_Universe.h"
#include "../../Utility/RC.h"
#include "../../Protocol/_SBus.h"

namespace kai
{

	struct APROVER_WB_MODE
	{
		int8_t MANUAL;
		int8_t FORWARD;
		int8_t BACKWARD;

		bool bValid(void)
		{
			IF_F(MANUAL < 0);
			IF_F(FORWARD < 0);
			IF_F(BACKWARD < 0);

			return true;
		}
	};

	class _AProver_WB : public _ModuleBase
	{
	public:
		_AProver_WB();
		~_AProver_WB();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);
		virtual void update(void);
		virtual void console(void *pConsole);

	protected:
		void updateMode(void);
		static void *getUpdate(void *This)
		{
			((_AProver_WB *)This)->update();
			return NULL;
		}

	public:
		_AP_base *m_pAP;
		_Drive *m_pD;
		RC_CHANNEL m_rcMode;
		APROVER_WB_MODE m_iMode;


		_SBus *m_pS;
		_ActuatorBase *m_pA;

		uint8_t m_iSpd; // channel index
		uint8_t m_iSteer;
		uint8_t m_iElev; // elevation bar

		float m_nSpd;
		float m_nSteer;
		float m_nElev;

	};

}
#endif
