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

	void onBtn(int key);
	static void callbackBtn(int key, void* pInst)
	{
		NULL_(pInst);
		_APcopter_sendPos* pS = (_APcopter_sendPos*)pInst;
		pS->onBtn(key);
	}

private:
	void updatePos(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_sendPos *) This)->update();
		return NULL;
	}

public:
	_APcopter_link* m_pOK;
	int		m_key;
	float	m_dP;
	vFloat3 m_vPrevP;
	float	m_dAlt;
	uint64_t m_tLastSent;
	uint64_t m_timeOut;

};

}
#endif
