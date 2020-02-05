#ifndef OpenKAI_src_Autopilot_AP__AP_link_H_
#define OpenKAI_src_Autopilot_AP__AP_link_H_

#include "../../../Protocol/_ProtocolBase.h"

namespace kai
{

#define APLINK_STATE 0
#define APLINK_BB 1
#define APLINK_ALT 2
#define APLINK_HDG 3
#define APLINK_TARGET 4

class _AP_link: public _ProtocolBase
{
public:
	_AP_link();
	~_AP_link();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void draw(void);

	void handleCMD(void);

	void state(uint8_t iState);
	void setBB(vFloat4& vP);
	void setTargetBB(vFloat4& vP);
	void setAlt(float dA);
	void setHdg(float dH);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_AP_link *) This)->update();
		return NULL;
	}

public:
	uint8_t		m_iState;
	uint64_t	m_tState;
	vFloat4		m_vBB;
	uint64_t	m_tBB;
	vFloat4		m_vTargetBB;
	uint64_t	m_tTargetBB;
	float		m_alt;
	uint64_t	m_tAlt;
	float		m_hdg;
	uint64_t	m_tHdg;
};

}
#endif
