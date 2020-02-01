
#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_servo_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_servo_H_

#include "_APcopter_base.h"

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

class _APcopter_servo: public _AutopilotBase
{
public:
	_APcopter_servo();
	~_APcopter_servo();

	bool init(void* pKiss);
	bool start(void);
	int	 check(void);
	void update(void);
	void draw(void);

private:
	void updateServo(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_servo*) This)->update();
		return NULL;
	}

private:
	_APcopter_base* m_pAP;
	vector<AP_SERVO> m_vServo;

};

}

#endif

