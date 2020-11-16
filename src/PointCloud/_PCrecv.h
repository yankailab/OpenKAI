/*
 * _PCrecv.h
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCrecv_H_
#define OpenKAI_src_PointCloud_PCrecv_H_

#include "../Base/common.h"
#include "../IO/_IOBase.h"
#include "../Protocol/_ProtocolBase.h"

#ifdef USE_OPEN3D
#include "_PCsend.h"
#include "_PCbase.h"

namespace kai
{
    
class _PCrecv: public _PCbase
{
public:
	_PCrecv();
	virtual ~_PCrecv();

	bool init(void* pKiss);
	bool start(void);
	int check(void);

private:
	virtual bool readCMD(void);
	virtual void handleCMD(void);
    void decodeStream(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_PCrecv *) This)->update();
		return NULL;
	}

public:
	_IOBase* m_pIO;
	PROTOCOL_CMD m_recvMsg;
	uint64_t	m_nCMDrecv;
};

}
#endif
#endif
