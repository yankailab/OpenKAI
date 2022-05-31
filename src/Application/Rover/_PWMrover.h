#ifndef OpenKAI_src_Autopilot_Rover__PWMrover_H_
#define OpenKAI_src_Autopilot_Rover__PWMrover_H_

#include "../../Vision/_DepthVisionBase.h"
#include "../../Universe/_Universe.h"
#include "../../Protocol/_PWMio.h"

namespace kai
{

	class _PWMrover : public _ModuleBase
	{
	public:
		_PWMrover();
		~_PWMrover();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);
		virtual void update(void);
		virtual void console(void *pConsole);
		virtual void draw(void *pFrame);

	protected:
		bool updateDrive(void);
		static void *getUpdate(void *This)
		{
			((_PWMrover *)This)->update();
			return NULL;
		}

	public:
		_DepthVisionBase *m_pDV;
		_Universe *m_pU;
		_PWMio *m_pPWM;

		int m_iClass;
		vInt2 m_vPWM;
		uint16_t m_pwmM;
		float m_targetX;
		float m_Kstr;
		float m_targetD;
		float m_Kspd;
		vFloat2 m_vKmotor;
		vInt2 m_vSpd;
		float m_minTarea;

		float m_d;
		uint16_t m_pwmL;
		uint16_t m_pwmR;
	};

}
#endif
