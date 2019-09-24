#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_sendPos_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_sendPos_H_

#include "../../../Base/common.h"
#include "../../_ActionBase.h"
#include "_APcopter_link.h"

namespace kai
{

class _APcopter_sendPos: public _ActionBase
{
public:
	_APcopter_sendPos();
	~_APcopter_sendPos();

	virtual	bool init(void* pKiss);
	virtual int check(void);
	virtual bool start(void);
	virtual void update(void);
	virtual bool draw(void);
	virtual bool console(int& iY);

	void onKey(int key);
	static void callbackKey(int key, void* pInst)
	{
		NULL_(pInst);
		_APcopter_sendPos* pS = (_APcopter_sendPos*)pInst;
		pS->onKey(key);
	}

private:
	void updateBB(void);
	void updateAlt(void);
	void updateHdg(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_sendPos *) This)->update();
		return NULL;
	}

public:
	_APcopter_link* m_pAL;
	float	m_diff;

	float	m_bbSize;
	float	m_dBBsize;
	vFloat2 m_vBBsize;
	vFloat4 m_vBB;

	float	m_alt;
	float	m_dAlt;
	float	m_hdg;
	float	m_dHdg;

	INTERVAL_EVENT m_ieSend;
};

}
#endif
