#pragma once

#include "../Base/common.h"
#include "../Camera/_CamStream.h"
#include "../Detector/_CascadeDetector.h"
#include "../Detector/_ObjectDetector.h"
#include "../Interface/_VehicleInterface.h"
#include "../Interface/_MavlinkInterface.h"
#include "../Tracker/_ROITracker.h"

#define NUM_RC_CHANNEL 8
#define NUM_CAM_STREAM 16

//FALCON COMMANDS
#define CMD_RC_UPDATE 0
#define CMD_OPERATE_MODE 1

#define TRD_INTERVAL_AUTOPILOT_USEC 10000

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


class _AutoPilot: public _ThreadBase
{
public:
	_AutoPilot();
	~_AutoPilot();

	bool init(JSON* pJson, string pilotName);
	bool start(void);

	void setVehicleInterface(_VehicleInterface* pVehicle);
	int* getPWMOutput(void);
	void resetAllControl(void);

	void camROILock(void);

	void remoteMavlinkMsg(MESSAGE* pMsg);

public:
	//Detectors
	_ObjectDetector*	m_pOD;
	_CascadeDetector*	m_pFD;
	_ROITracker* 		m_pROITracker;

	_VehicleInterface* m_pVI;
	_MavlinkInterface* m_pMavlink;

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


/*
#include "../Base/common.h"
#include "../Camera/_CamStream.h"
#include "../Detector/_CascadeDetector.h"
#include "../Detector/_MarkerDetector.h"
#include "../Detector/_ObjectDetector.h"
#include "../Interface/_VehicleInterface.h"

//
// Operation mode
//
#define OPE_SERIAL_BRIDGE 0
#define OPE_MANUAL 1
#define OPE_ALT_HOLD 2
#define OPE_LOITER 3
#define OPE_AUTO 4
#define OPE_RC_BRIDGE 5
#define OPE_BOOT 6

#define NUM_RC_CHANNEL 8
#define NUM_CAM_STREAM 16

//FALCON COMMANDS
#define CMD_RC_UPDATE 0
#define CMD_OPERATE_MODE 1


#define CAM_FRONT 0
#define CAM_LEFT 1
#define CAM_RIGHT 2
#define CAM_BACK 3

#define TRD_INTERVAL_AUTOPILOT_USEC 10000

namespace kai
{

struct PID_SETTING
{
	double m_P;
	double m_I;
	double m_D;
	double m_Z;	//Depth value for the setting
};

struct CONTROL_AXIS
{
	double m_pos;
	double m_targetPos;
	double m_accel;

	double m_cvErr;
	double m_cvErrOld;
	double m_cvErrInteg;

	double m_P;
	double m_I;
	double m_Imax;
	double m_D;

	unsigned int m_pwm;
	unsigned int m_pwmLow;
	unsigned int m_pwmCenter;
	unsigned int m_pwmHigh;
	unsigned int m_RCChannel;
};

struct CAMERA_STREAM
{
	uint64_t m_frameID;
	_CamStream*	m_pCam;
};

class _AutoPilot: public _ThreadBase
{
public:
	_AutoPilot();
	~_AutoPilot();

	bool setup(JSON* pJson, string pilotName);
	bool init(void);
	bool start(void);

	bool setCamStream(_CamStream* pCamStream, int camPosition);
	void markerLock(_MarkerDetector* pCMD);
	void flowLock(_MarkerDetector* pCMD);
	void setTargetPosCV(fVector3 pos);
	fVector3 getTargetPosCV(void);

	void setVehicleInterface(_VehicleInterface* pVehicle);

	int* getPWMOutput(void);

	void remoteMavlinkMsg(MESSAGE* pMsg);
	SYSTEM m_hostSystem;
	SYSTEM m_remoteSystem;
	MESSAGE* m_pRecvMsg;

public:
	//Camera Stream
	int 			m_numCamStream;
	CAMERA_STREAM	m_pCamStream[NUM_CAM_STREAM];

	_ObjectDetector*	m_pOD;
	_CascadeDetector*	m_pFD;

	_VehicleInterface* m_pVI;

	//Common
	double m_dT;

	//Marker Lock
	int m_lockLevel;
	bool m_bAutoCalcDelay;
	fVector3 m_cvPos;
	fVector3 m_cvPosOld;
	fVector3 m_cvAtt;

	//Flow Lock
	int m_iFlowFrame;
	int m_resetFlowFrame;
	double m_resetFactor;
	fVector4 m_flowTotal;

	//Control
	CONTROL_AXIS m_roll;
	CONTROL_AXIS m_pitch;
	CONTROL_AXIS m_yaw;
	CONTROL_AXIS m_alt;

	PID_SETTING m_rollFar;
	PID_SETTING m_rollNear;
	PID_SETTING m_altFar;
	PID_SETTING m_altNear;
	PID_SETTING m_pitchFar;
	PID_SETTING m_pitchNear;
	PID_SETTING m_yawFar;
	PID_SETTING m_yawNear;

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

 *
 */
