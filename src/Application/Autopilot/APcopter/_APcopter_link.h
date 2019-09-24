#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_link_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_link_H_

#include "../../../Protocol/_ProtocolBase.h"

namespace kai
{

#define APLINK_STATE 0
#define APLINK_BB 1
#define APLINK_ALT 2
#define APLINK_HDG 3

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

	void state(uint8_t iState);
	void setBB(vFloat4& vP);
	void setAlt(float dA);
	void setHdg(float dH);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_link *) This)->update();
		return NULL;
	}

public:
	vFloat4		m_vBB;
	uint64_t	m_tBB;
	uint8_t		m_iState;
	uint64_t	m_tState;
	float		m_alt;
	uint64_t	m_tAlt;
	float		m_hdg;
	uint64_t	m_tHdg;
};

}
#endif
