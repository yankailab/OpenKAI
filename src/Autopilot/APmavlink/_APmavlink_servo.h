
#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_servo_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_servo_H_

#include "_APmavlink_base.h"

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

	class _APmavlink_servo : public _ModuleBase
	{
	public:
		_APmavlink_servo();
		~_APmavlink_servo();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void update(void);
		virtual void console(void *pConsole);

	private:
		void updateServo(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_servo *)This)->update();
			return NULL;
		}

	protected:
		_APmavlink_base *m_pAP;
		vector<AP_SERVO> m_vServo;
	};

}

#endif
