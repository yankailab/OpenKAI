#ifndef OpenKAI_src_Protocol___UIhandlerBase_H_
#define OpenKAI_src_Protocol___UIhandlerBase_H_

#include "../../Protocol/_JSONbase.h"
#include "../../IO/_WebSocket.h"

using namespace picojson;

namespace kai
{

class _UIhandlerBase : public _JSONbase
{
public:
	_UIhandlerBase();
	~_UIhandlerBase();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

protected:
	virtual void send(void);
	virtual bool recv(void);
	virtual void handleMsg(string& str);

private:
	void updateW(void);
	static void* getUpdateW(void* This)
	{
		((_UIhandlerBase*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateR(void* This)
	{
		((_UIhandlerBase*) This)->updateR();
		return NULL;
	}

public:

};

}
#endif
