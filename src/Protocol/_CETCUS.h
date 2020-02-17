#ifndef OpenKAI_src_Protocol__CETCUS_H_
#define OpenKAI_src_Protocol__CETCUS_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../IO/_TCPclient.h"

#define CETCUS_N_BUF 1024

namespace kai
{

class _CETCUS: public _ThreadBase
{
public:
	_CETCUS();
	~_CETCUS();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void draw(void);

	void planeConnect(void);

	bool recv();

public:
	void handleJSON(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_CETCUS *) This)->update();
		return NULL;
	}

	_TCPclient* m_pIO;

	string m_uavNo;

	vector<JSON> m_vJSON;

};

}

#endif
