#ifndef OpenKAI_src_State__StateBase_H_
#define OpenKAI_src_State__StateBase_H_

#include "_StateControl.h"

namespace kai
{

class _StateBase: public _ModuleBase
{
public:
	_StateBase();
	~_StateBase();

	virtual bool init(void* pKiss);
	virtual void update(void);
	virtual bool bActive(void);
	virtual bool bStateChanged(void);
	virtual int check(void);
	virtual void console(void* pConsole);
	virtual void msgActive(void* pConsole);

public:
	_StateControl* m_pSC;
	std::vector<int> m_vActiveState;
	int m_iLastState;
	bool m_bStateChanged;
};

}
#endif
