/*
 * _HiphenServer.h
 *
 *  Created on: Feb 19, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__HiphenServer_H_
#define OpenKAI_src_Vision__HiphenServer_H_

#include "../../Base/common.h"
#include "../../IO/_TCPserver.h"
#include "_HiphenIMG.h"

namespace kai
{

class _HiphenServer: public _TCPserver
{
public:
	_HiphenServer();
	virtual ~_HiphenServer();

	bool init(void* pKiss);
	bool start(void);

	bool handler(void);
	void cleanupClient(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_HiphenServer*) This)->update();
		return NULL;
	}

public:
	string m_dir;
	string m_subDir;

};

}
#endif
