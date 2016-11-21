/*
 * _server.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef SRC_NETWORK_SERVERE_H_
#define SRC_NETWORK_SERVERE_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Script/Kiss.h"
#include "NetworkBase.h"


namespace kai
{

class _server: public _ThreadBase, public NetworkBase
{
public:
	_server();
	virtual ~_server();

	bool init(Kiss* pKiss);
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
