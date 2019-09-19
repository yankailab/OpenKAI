#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_link_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_link_H_

#include "../../../Protocol/_ProtocolBase.h"

namespace kai
{

#define APLINK_POS 3

class _APcopter_link: public _ProtocolBase
{
public:
	_APcopter_link();
	~_APcopter_link();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);

	void handleCMD(void);

	void setPos(vFloat3 vP);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_link *) This)->update();
		return NULL;
	}

public:
	vFloat3		m_vPos;
	uint64_t	m_tPos;
};

}
#endif
