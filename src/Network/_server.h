/*
 * _server.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef SRC_NETWORK_SERVERE_H_
#define SRC_NETWORK_SERVERE_H_

#include "NetworkBase.h"
#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Base/cv.h"

namespace kai
{

class _server: public _ThreadBase, public NetworkBase
{
public:
	_server();
	virtual ~_server();

	bool init(JSON* pJson);
	bool start(void);

private:
	void update(void);
	static void* getUpdateThread(void* This) {
		((_server*) This)->update();
		return NULL;
	}

public:
	uint64_t m_frameID;

};

} /* namespace kai */

#endif /* SRC_NETWORK_SERVERE_H_ */
