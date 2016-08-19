#pragma once

#include "../Base/common.h"
#include "../Camera/_Stream.h"
#include "../Detector/_CascadeDetector.h"
#include "../Detector/_MarkerDetector.h"
#include "../Interface/_VehicleInterface.h"
#include "../Interface/_MavlinkInterface.h"
#include "../Tracker/_ROITracker.h"

#define NUM_RC_CHANNEL 8
#define NUM_CAM_STREAM 16

//FALCON COMMANDS
#define CMD_RC_UPDATE 0
#define CMD_OPERATE_MODE 1

namespace kai
{

struct CONTROL_PID
{
	double m_P;
	double m_I;
	double m_Imax;
	double m_D;
	double m_dT;
};

struct RC_CHANNEL
{
	int m_pwm;
	int m_pwmLow;
	int m_pwmCenter;
	int m_pwmHigh;
	int m_idx;
};

struct CONTROL_CHANNEL
{
	double m_pos;
	double m_targetPos;

	double m_err;
	double m_errOld;
	double m_errInteg;

	CONTROL_PID m_pid;
	RC_CHANNEL m_RC;
};

struct LANDING_TARGET
{
	double m_angleX;
	double m_angleY;
	double m_orientX;
	double m_orientY;
	uint64_t m_ROIstarted;
	uint64_t m_ROItimeLimit;

};


class _AutoPilot: public _ThreadBase
{
public:
	_AutoPilot();
	~_AutoPilot();

	bool init(JSON* pJson, string pilotName);
	bool start(void);

	void setVehicleInterface(_VehicleInterface* pVehicle);
	void setMavlinkInterface(_MavlinkInterface* pMavlink);
	int* getPWMOutput(void);
	void resetAllControl(void);

	void camROILock(void);
	void camMarkerLock(void);

	void remoteMavlinkMsg(MESSAGE* pMsg);

public:
	//Detectors
	_CascadeDetector*	m_pFD;
	_ROITracker* 		m_pROITracker;
	_MarkerDetector*	m_pMarkerDetector;

	_VehicleInterface* m_pVI;
	_MavlinkInterface* m_pMavlink;

	LANDING_TARGET	m_landingTarget;

	//Control
	CONTROL_CHANNEL m_roll;
	CONTROL_CHANNEL m_pitch;
	CONTROL_CHANNEL m_yaw;
	CONTROL_CHANNEL m_alt;

	int m_RC[NUM_RC_CHANNEL];

	MESSAGE* m_pRecvMsg;

	//Thread
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_AutoPilot *) This)->update();
		return NULL;
	}

};

}


