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

#ifdef USE_OPENCV
#ifdef USE_OPEN3D
#include "_PCbase.h"

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
	void updateIO(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_PCsend *) This)->update();
		return NULL;
	}

public:
	_PCbase* m_pPCB;
	_IOBase* m_pIO;

};

}
#endif
#endif
#endif
