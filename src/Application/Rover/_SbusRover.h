#ifndef OpenKAI_src_Autopilot_Rover__SbusRover_H_
#define OpenKAI_src_Autopilot_Rover__SbusRover_H_

#include "../../Vision/_DepthVisionBase.h"
#include "../../Universe/_Universe.h"
#include "../../Protocol/_SBus.h"
#include "../../Autopilot/Drive/_Drive.h"

namespace kai
{

	class _SbusRover : public _StateBase
	{
	public:
		_SbusRover();
		~_SbusRover();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void update(void);
		virtual void console(void *pConsole);
		virtual void draw(void *pFrame);

	protected:
		bool findTarget(_Object *pObj);
		bool updateDrive(void);
		static void *getUpdate(void *This)
		{
			((_SbusRover *)This)->update();
			return NULL;
		}

	public:
		_SBus *m_pSbus;
		_Drive *m_pD;
		_ActuatorBase *m_pA;

		// channel index
		uint8_t m_iMode;
		uint8_t m_iSpd;
		uint8_t m_iSteer;
		uint8_t m_iElev; // elevation bar
		vInt2 m_vModeR;	 // raw pwm range for mode
		float m_nSpd;
		float m_nSteer;
		float m_nElev;

		_DepthVisionBase *m_pDV;
		_Universe *m_pU;

		int m_iClass;
		float m_minTarea;
		float m_targetX;
		float m_Kstr;
		float m_spdFollow;
		float m_tAreaStop;
		vFloat4 m_vTbb;
		float m_dTarget;
		float m_dTargetStop;
	};

}
#endif
