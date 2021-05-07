#ifndef OpenKAI_src_Application_3Dscan__MultiRsScan_H_
#define OpenKAI_src_Application_3Dscan__MultiRsScan_H_

#ifdef USE_OPEN3D
#include "../../Protocol/_JSONbase.h"
#include "../../IO/_WebSocket.h"
#include "../../PointCloud/_PCbase.h"

using namespace picojson;

namespace kai
{

class _MultiRsScan : public _JSONbase
{
public:
	_MultiRsScan();
	~_MultiRsScan();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void console(void* pConsole);

protected:
	void send(void);
	bool recv(void);
	void handleMsg(string& str);

private:
	virtual _PCbase* findPC(string& n);
	void updateW(void);
	static void* getUpdateW(void* This)
	{
		(( _MultiRsScan*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateR(void* This)
	{
		(( _MultiRsScan*) This)->updateR();
		return NULL;
	}

public:
    _Thread* m_pTr;
    
    vector<_PCbase*> m_vPCB;

};

}
#endif
#endif
