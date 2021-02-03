/*
 * _PCsend.h
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCsend_H_
#define OpenKAI_src_PointCloud_PCsend_H_

#include "../Base/common.h"
#include "../IO/_IOBase.h"
#include "../Protocol/_ProtocolBase.h"

#ifdef USE_OPEN3D
#include "_PCbase.h"

#define PC_N_HDR 4
#define PC_STREAM 0
#define PC_FRAME_END 1

namespace kai
{

class _PCsend: public _PCbase
{
public:
	_PCsend();
	virtual ~_PCsend();

	bool init(void* pKiss);
	bool start(void);
	int check(void);

private:
	void sendPC(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_PCsend *) This)->update();
		return NULL;
	}

public:
	_IOBase* m_pIO;

	uint8_t*	m_pB;
	int			m_nB;
    uint64_t    m_tInt;
};

}
#endif
#endif
