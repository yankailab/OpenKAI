
#ifndef OpenKAI_src_Autopilot_AP__AP_relay_H_
#define OpenKAI_src_Autopilot_AP__AP_relay_H_

#include "../ArduPilot/_AP_base.h"

namespace kai
{

struct AP_relay
{
	uint8_t m_iChan;
	bool m_bRelay;

	void init(void)
	{
		m_iChan = 9;
		m_bRelay = false;
	}
};

class _AP_relay: public _StateBase
{
public:
	_AP_relay();
	~_AP_relay();

	bool init(void* pKiss);
	bool start(void);
	int	 check(void);
	void update(void);
	void draw(void);

private:
	void updateRelay(void);
	static void* getUpdate(void* This)
	{
		((_AP_relay*) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	vector<AP_relay> m_vRelay;

};

}

#endif

