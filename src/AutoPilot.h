#pragma once

#include "common.h"
#include "CamStream.h"
#include "ObjectDetector.h"
#include "FastDetector.h"
#include "util.h"
#include "VehicleInterface.h"

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
	int			m_frameID;
	CamStream*	m_pCam;
};

class AutoPilot: public ThreadBase
{
public:
	AutoPilot();
	~AutoPilot();

	bool init(void);
	bool start(void);
	bool complete(void);
	void stop(void);
	void waitForComplete(void);

	bool setCamStream(CamStream* pCamStream, int camPosition);
	void markerLock(CamMarkerDetect* pCMD);
	void flowLock(CamMarkerDetect* pCMD);
	void setTargetPosCV(fVector3 pos);
	fVector3 getTargetPosCV(void);

	void setVehicleInterface(VehicleInterface* pVehicle);

	int* getPWMOutput(void);

	void remoteMavlinkMsg(MESSAGE* pMsg);
	SYSTEM m_hostSystem;
	SYSTEM m_remoteSystem;
	MESSAGE* m_pRecvMsg;

public:
	//Camera Stream
	int 				m_numCamStream;
	CAMERA_STREAM	m_pCamStream[NUM_CAM_STREAM];

	ObjectDetector*	m_pOD;
	FastDetector*	m_pFD;

	VehicleInterface* m_pVI;



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
	pthread_t m_threadID;
	bool m_bThreadON;

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((AutoPilot *) This)->update();
		return NULL;
	}

};

}

