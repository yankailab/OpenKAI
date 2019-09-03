#ifndef OpenKAI_src_Autopilot_APcopter__OKlinkAPcopter_H_
#define OpenKAI_src_Autopilot_APcopter__OKlinkAPcopter_H_

#include "../../../Protocol/_OKlink.h"

namespace kai
{

class _OKlinkAPcopter: public _OKlink
{
public:
	_OKlinkAPcopter();
	~_OKlinkAPcopter();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);

	void handleCMD(void);

	void setPos(vFloat2 vP);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_OKlinkAPcopter *) This)->update();
		return NULL;
	}

public:
	vFloat2		m_vPos;
	uint64_t	m_tPos;
};

}
#endif
