/*
 * _Pylon.h
 *
 *  Created on: Apr 9, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Pylon_H_
#define OpenKAI_src_Vision__Pylon_H_

#include "../Base/common.h"
#include "_VisionBase.h"

#ifdef USE_PYLON
#include <pylon/PylonIncludes.h>

using namespace Pylon;

namespace kai
{

class _Pylon: public _VisionBase
{
public:
	_Pylon();
	virtual ~_Pylon();

	bool init(void* pKiss);
	bool start(void);

private:
	bool open(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Pylon*) This)->update();
		return NULL;
	}

public:
	PylonAutoInitTerm m_pylonAutoInit;
	CInstantCamera* m_pPylonCam;
	CImageFormatConverter m_pylonFC;
	CPylonImage m_pylonImg;
	CGrabResultPtr m_pylonGrab;

	string m_SN;
	int	m_grabTimeout;

};

}
#endif
#endif
