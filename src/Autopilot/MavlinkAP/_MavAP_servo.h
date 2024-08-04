
#ifndef OpenKAI_src_Autopilot_MavAP__MavAP_servo_H_
#define OpenKAI_src_Autopilot_MavAP__MavAP_servo_H_

#include "_MavAP_base.h"

namespace kai
{

	struct AP_SERVO
	{
		uint8_t m_iChan;
		uint16_t m_pwm;

		void init(void)
		{
			m_iChan = 9;
			m_pwm = 1500;
		}
	};

	class _MavAP_servo : public _ModuleBase
	{
	public:
		_MavAP_servo();
		~_MavAP_servo();

		int init(void *pKiss);
		int start(void);
		int check(void);
		void update(void);
		void console(void *pConsole);

	private:
		void updateServo(void);
		static void *getUpdate(void *This)
		{
			((_MavAP_servo *)This)->update();
			return NULL;
		}

	private:
		_MavAP_base *m_pAP;
		vector<AP_SERVO> m_vServo;
	};

}

#endif
