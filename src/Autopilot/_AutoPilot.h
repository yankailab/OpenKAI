#pragma once

#include "../Base/common.h"
#include "../Detector/_Bullseye.h"
#include "../Detector/_AprilTags.h"
#include "../Stream/_Stream.h"
#include "../Detector/_Cascade.h"
#include "../Interface/_Mavlink.h"
#include "../Interface/_RC.h"
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
	bool draw(Frame* pFrame, iVector4* pTextPos);
	void sendHeartbeat(void);

	void setVehicleInterface(_RC* pVehicle);
	void setMavlinkInterface(_Mavlink* pMavlink);
	int* getPWMOutput(void);
	void resetAllControl(void);

	void camROILock(void);
	void landingTarget(void);

public:
	//Detectors
	_Cascade*		m_pFD;
	_ROITracker* 	m_pROITracker;
	_Bullseye*		m_pMD;

	_AprilTags*		m_pAT;
	APRIL_TAG		m_pATags[NUM_PER_TAG];
	int				m_pATagsLandingTarget[NUM_PER_TAG];
	int				m_numATagsLandingTarget;

	_RC* m_pVI;
	_Mavlink* m_pMavlink;

	uint64_t m_lastHeartbeat;
	uint64_t m_iHeartbeat;

	LANDING_TARGET	m_landingTarget;

	//Control
	CONTROL_CHANNEL m_roll;
	CONTROL_CHANNEL m_pitch;
	CONTROL_CHANNEL m_yaw;
	CONTROL_CHANNEL m_alt;

	int m_RC[NUM_RC_CHANNEL];

	//Thread
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_AutoPilot *) This)->update();
		return NULL;
	}

};

}


