/*
 * _HiphenCMD.h
 *
 *  Created on: Feb 19, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__HiphenCMD_H_
#define OpenKAI_src_Vision__HiphenCMD_H_

#include "../../Base/common.h"
#include "../../IO/_TCPclient.h"

namespace kai
{

class _HiphenCMD: public _TCPclient
{
public:
	_HiphenCMD();
	virtual ~_HiphenCMD();

	bool init(void* pKiss);
	bool start(void);

	void getID(void);
	void getStatus(void);
	void getTime(void);
	void getBatteryLevel(void);
	void getGPScoordinates(void);
	void getImageInfo(void);
	void setImageInfo(string& opr, string& imgDesc);
	void getTriggerSettings(void);
	void setTriggerSettings(string& tIn, string& tOut);
	void getExposureMode(void);
	void setExposureMode(string& mode);
	void getIntegrationTime(int iSensor);
	void setIntegrationTime(int iSensor, int tInt);
	void getGainMode(void);
	void setGainMode(string& mode);
	void getGain(int iSensor);
	void setGain(int iSensor, int gain);
	void startRecord(void);
	void stopRecord(void);
	void snapshot(int nSnapshot, string& mode);

public:
	void updateW(void);
	static void* getUpdateW(void* This)
	{
		((_HiphenCMD*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateR(void* This)
	{
		((_HiphenCMD*) This)->updateR();
		return NULL;
	}

public:
    _Thread* m_pTr;

};

}
#endif
