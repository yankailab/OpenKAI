#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_sendPos_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_sendPos_H_

#include "../../../Base/common.h"
#include "../../_ActionBase.h"
#include "_OKlinkAPcopter.h"

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

private:
	void updatePos(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_sendPos *) This)->update();
		return NULL;
	}

public:
	_OKlinkAPcopter* m_pOK;

};

}
#endif
