#ifndef OpenKAI_src_Autopilot_Controller_APcopter_base_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_base_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Mavlink.h"
#include "../../../Control/PIDctrl.h"
#include "../../ActionBase.h"

namespace kai
{

class APcopter_base: public ActionBase
{
public:
	APcopter_base();
	~APcopter_base();

	bool init(void* pKiss);
	int check(void);
	void update(void);
	bool draw(void);
	bool cli(int& iY);

	void setApMode(uint32_t iMode);
	uint32_t getApMode(void);
	void setApArm(bool bArm);
	bool getApArm(void);
	bool bApModeChanged(void);
	uint32_t apMode(void);

public:
	_Mavlink* m_pMavlink;
	uint64_t m_lastHeartbeat;
	uint64_t m_iHeartbeat;

	uint32_t m_apMode;
	uint32_t m_lastApMode;
	bool	 m_bApModeChanged;

	int m_freqAtti;
	int m_freqGlobalPos;
	int m_freqHeartbeat;
	int m_freqRC;

};

}
#endif
