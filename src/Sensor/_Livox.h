/*
 * _Livox.h
 *
 *  Created on: Aug 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__Livox_H_
#define OpenKAI_src_Sensor__Livox_H_

#include "../Base/_ThreadBase.h"

#ifdef USE_LIVOX
#include "livox_sdk.h"

namespace kai
{

typedef enum
{
	kDeviceStateDisconnect = 0,
	kDeviceStateConnect = 1,
	kDeviceStateSampling = 2,
} LivoxDeviceState;

typedef struct
{
	uint8_t handle;
	LivoxDeviceState device_state;
	DeviceInfo info;
} LivoxDevice;

class _Livox: public _ThreadBase
{
public:
	_Livox();
	~_Livox();

	bool init(void* pKiss);
	void close(void);
	bool start(void);
	void draw(void);

private:
	bool open(void);
	bool updateLidar(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Livox*) This)->update();
		return NULL;
	}

public:
	bool m_bOpen;

};

}
#endif
#endif
