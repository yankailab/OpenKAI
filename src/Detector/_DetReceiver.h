/*
 * _DetReceiver.h
 *
 *  Created on: Sept 3, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__DetReceiver_H_
#define OpenKAI_src_Detector__DetReceiver_H_

#include "../Base/common.h"
#include "_DetectorBase.h"
#include "../Application/Autopilot/APcopter/_APcopter_link.h"

namespace kai
{

class _DetReceiver : public _DetectorBase
{
public:
	_DetReceiver();
	virtual ~_DetReceiver();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	bool console(int& iY);
	bool draw(void);

private:
	void receive(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DetReceiver*) This)->update();
		return NULL;
	}

public:
	_APcopter_link* m_pOK;
	uint64_t	m_timeOut;

};
}

#endif
