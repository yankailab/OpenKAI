/*
 * _HiphenIMG.h
 *
 *  Created on: Feb 19, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__HiphenIMG_H_
#define OpenKAI_src_Vision__HiphenIMG_H_

#include "../Base/common.h"
#include "../IO/_TCPserver.h"

namespace kai
{

class _HiphenIMG: public _TCPserver
{
public:
	_HiphenIMG();
	virtual ~_HiphenIMG();

	bool init(void* pKiss);
	bool start(void);

private:
	bool handler(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_HiphenIMG*) This)->update();
		return NULL;
	}

public:

};

}

#endif
