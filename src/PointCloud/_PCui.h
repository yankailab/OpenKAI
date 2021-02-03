#ifndef OpenKAI_src_Protocol___PCwebUI_H_
#define OpenKAI_src_Protocol___PCwebUI_H_

#include "../Protocol/_JSONbase.h"
#include "../IO/_WebSocket.h"
#include "_PCtransform.h"

using namespace picojson;

#ifdef USE_OPEN3D
namespace kai
{

class _PCui : public _JSONbase
{
public:
	_PCui();
	~_PCui();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

protected:
	virtual void send(void);
	virtual bool recv(void);
	virtual void handleMsg(string& str);

private:
	virtual _PCtransform* findTransform(string& n);
	void updateW(void);
	static void* getUpdateW(void* This)
	{
		((_PCui*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateR(void* This)
	{
		((_PCui*) This)->updateR();
		return NULL;
	}

public:
    vector<_PCtransform*> m_vPCT;
    

};

}
#endif
#endif
