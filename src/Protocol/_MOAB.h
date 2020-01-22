#ifndef OpenKAI_src_Protocol__MOAB_H_
#define OpenKAI_src_Protocol__MOAB_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../IO/_IOBase.h"

namespace kai
{

class _MOAB: public _ThreadBase
{
public:
	_MOAB();
	~_MOAB();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);

	void setSpeed(float speedL, float speedR);

private:
	void update(void);
	void send(void);
	static void* getUpdateThread(void* This)
	{
		((_MOAB *) This)->update();
		return NULL;
	}

public:
	_IOBase* m_pIO;

	float m_speedL;
	float m_speedR;

};

}

#endif
