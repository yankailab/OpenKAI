/*
 * _HiphenCMD.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__HiphenCMD_H_
#define OpenKAI_src_Vision__HiphenCMD_H_

#include "../Base/common.h"
#include "../IO/_TCPclient.h"

namespace kai
{

class _HiphenCMD: public _TCPclient
{
public:
	_HiphenCMD();
	virtual ~_HiphenCMD();

	bool init(void* pKiss);
	bool start(void);

private:
	void updateW(void);
	static void* getUpdateThreadW(void* This)
	{
		((_HiphenCMD*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateThreadR(void* This)
	{
		((_HiphenCMD*) This)->updateR();
		return NULL;
	}

public:
	int m_nSave;
	int m_nSnap;

};

}

#endif
