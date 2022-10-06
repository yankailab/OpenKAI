#ifndef OpenKAI_src_Autopilot_Rover__RCrover_H_
#define OpenKAI_src_Autopilot_Rover__RCrover_H_

#include "../../Protocol/_SBus.h"
#include "../../Autopilot/Drive/_Drive.h"

namespace kai
{

	class _RCrover : public _StateBase
	{
	public:
		_RCrover();
		~_RCrover();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);
		virtual void update(void);
		virtual void console(void *pConsole);

	protected:
		bool updateDrive(void);
		static void *getUpdate(void *This)
		{
			((_RCrover *)This)->update();
			return NULL;
		}

	public:
		_SBus *m_pS;
		_Drive *m_pD;
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
