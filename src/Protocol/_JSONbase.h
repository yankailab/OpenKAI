#ifndef OpenKAI_src_Protocol__JSONbase_H_
#define OpenKAI_src_Protocol__JSONbase_H_

#include "../Base/_ThreadBase.h"
#include "../IO/_IOBase.h"
#include <openssl/md5.h>

using namespace picojson;

namespace kai
{

class _JSONbase: public _ThreadBase
{
public:
	_JSONbase();
	~_JSONbase();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

protected:
	virtual void send(void);
    virtual bool sendMsg (picojson::object& o);
    virtual bool sendHeartbeat (void);    

	virtual bool recv(void);
	virtual void handleMsg(string& str);
    virtual void md5(string& str, string* pDigest);
   	virtual bool str2JSON(string& str, picojson::value* pJson);

private:
	void updateW(void);
	static void* getUpdateThreadW(void* This)
	{
		((_JSONbase*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateThreadR(void* This)
	{
		((_JSONbase*) This)->updateR();
		return NULL;
	}

public:
	_IOBase* m_pIO;

	string m_msgFinishSend;
	string m_msgFinishRecv;
    string m_strB;

	pthread_t m_rThreadID;
	bool	m_bRThreadON;

    INTERVAL_EVENT m_tIntHeartbeat;
};

}
#endif
