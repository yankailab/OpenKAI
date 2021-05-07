
#ifndef OpenKAI_src_Autopilot_AP__AP_servo_H_
#define OpenKAI_src_Autopilot_AP__AP_servo_H_

#include "../ArduPilot/_AP_base.h"

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

class _AP_servo: public _StateBase
{
public:
	_AP_servo();
	~_AP_servo();

	bool init(void* pKiss);
	bool start(void);
	int	 check(void);
	void update(void);
	void console(void* pConsole);

private:
	void updateServo(void);
	static void* getUpdate(void* This)
	{
		((_AP_servo*) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	vector<AP_SERVO> m_vServo;

};

}

#endif

