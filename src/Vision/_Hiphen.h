/*
 * _Hiphen.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Hiphen_H_
#define OpenKAI_src_Vision__Hiphen_H_

#include "../Base/common.h"
#include "../IO/_TCPclient.h"
#include "_VisionBase.h"

namespace kai
{

class _Hiphen: public _VisionBase
{
public:
	_Hiphen();
	virtual ~_Hiphen();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);

	void snapshot(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Hiphen*) This)->update();
		return NULL;
	}

public:
	_TCPclient* m_pTCP;
	int m_nSave;
	int m_nSnap;

};

}

#endif
