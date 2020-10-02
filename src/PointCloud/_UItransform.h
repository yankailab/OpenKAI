#ifndef OpenKAI_src_Protocol__UItransform_H_
#define OpenKAI_src_Protocol__UItransform_H_

#include "../Base/_ThreadBase.h"
#include "../IO/_WebSocket.h"
#include "_PCtransform.h"

using namespace picojson;

namespace kai
{

class _UItransform: public _ThreadBase
{
public:
	_UItransform();
	~_UItransform();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void draw(void);

private:
	//receive
	bool recv();
	void handleMsg(string& str);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_UItransform*) This)->update();
		return NULL;
	}

public:
	_IOBase* m_pIO;
	vector<_PCtransform*> m_vPCT;

};

}
#endif
