/*
 * Config.cpp
 *
 *  Created on: Aug 24, 2015
 *      Author: yankai
 */

#include "Config.h"

namespace kai
{

Config::Config()
{
	m_pJson = NULL;

}

Config::~Config()
{
	// TODO Auto-generated destructor stub
}


bool Config::setJSON(JSON* pJson)
{
	if(!pJson)return false;

	m_pJson = pJson;

	return true;
}

bool Config::setCamStream(CamStream* pCamStream)
{
	if(!m_pJson)
	{
		return false;
	}

	if(!pCamStream)
	{
		return false;
	}

	CHECK_FATAL(m_pJson->getVal("CAM_FRONT_NAME", &pCamStream->m_camName));
	CHECK_FATAL(m_pJson->getVal("CAM_FRONT_ID_L", &pCamStream->m_pCamL->m_camDeviceID));

	CHECK_FATAL(m_pJson->getVal("CAM_FRONT_WIDTH", &pCamStream->m_pCamL->m_width));
	CHECK_FATAL(m_pJson->getVal("CAM_FRONT_HEIGHT", &pCamStream->m_pCamL->m_height));

	int bSwitch;
	if(m_pJson->getVal("CAM_FRONT_MARKER", &bSwitch))
	{
		if(bSwitch)
		{
			pCamStream->m_bHSV = true;
			pCamStream->m_bMarkerDetect = true;
		}
	}


	return true;

}

bool Config::setAutoPilot(AutoPilot* pAuto)
{
	if(!m_pJson)
	{
		return false;
	}

	if(!pAuto)
	{
		return false;
	}


	CONTROL_AXIS cAxis;

	cAxis.m_pos = 0;
	cAxis.m_targetPos = 0;
	cAxis.m_accel = 0;
	cAxis.m_cvErr = 0;
	cAxis.m_cvErrOld = 0;
	cAxis.m_cvErrInteg = 0;
	cAxis.m_P = 0;
	cAxis.m_I = 0;
	cAxis.m_Imax = 0;
	cAxis.m_D = 0;
	cAxis.m_pwm = 0;
	cAxis.m_pwmCenter = 0;
	cAxis.m_RCChannel = 0;

	pAuto->m_roll = cAxis;
	pAuto->m_pitch = cAxis;
	pAuto->m_alt = cAxis;

	CHECK_ERROR(m_pJson->getVal("ROLL_P", &pAuto->m_roll.m_P));
	CHECK_ERROR(m_pJson->getVal("ROLL_I", &pAuto->m_roll.m_I));
	CHECK_ERROR(m_pJson->getVal("ROLL_IMAX", &pAuto->m_roll.m_Imax));
	CHECK_ERROR(m_pJson->getVal("ROLL_D", &pAuto->m_roll.m_D));
	CHECK_ERROR(m_pJson->getVal("PWM_CENTER", (int*)&pAuto->m_roll.m_pwm));
	CHECK_ERROR(m_pJson->getVal("PWM_LOW", (int*)&pAuto->m_roll.m_pwmLow));
	CHECK_ERROR(m_pJson->getVal("PWM_HIGH", (int*)&pAuto->m_roll.m_pwmHigh));
	CHECK_ERROR(m_pJson->getVal("PWM_CENTER", (int*)&pAuto->m_roll.m_pwmCenter));
	CHECK_ERROR(m_pJson->getVal("RC_ROLL", (int*)&pAuto->m_roll.m_RCChannel));

	CHECK_ERROR(m_pJson->getVal("PITCH_P", &pAuto->m_pitch.m_P));
	CHECK_ERROR(m_pJson->getVal("PITCH_I", &pAuto->m_pitch.m_I));
	CHECK_ERROR(m_pJson->getVal("PITCH_IMAX", &pAuto->m_pitch.m_Imax));
	CHECK_ERROR(m_pJson->getVal("PITCH_D", &pAuto->m_pitch.m_D));
	CHECK_ERROR(m_pJson->getVal("PWM_CENTER", (int*)&pAuto->m_pitch.m_pwm));
	CHECK_ERROR(m_pJson->getVal("PWM_LOW", (int*)&pAuto->m_pitch.m_pwmLow));
	CHECK_ERROR(m_pJson->getVal("PWM_HIGH", (int*)&pAuto->m_pitch.m_pwmHigh));
	CHECK_ERROR(m_pJson->getVal("PWM_CENTER", (int*)&pAuto->m_pitch.m_pwmCenter));
	CHECK_ERROR(m_pJson->getVal("RC_PITCH", (int*)&pAuto->m_pitch.m_RCChannel));

	CHECK_ERROR(m_pJson->getVal("ALT_P", &pAuto->m_alt.m_P));
	CHECK_ERROR(m_pJson->getVal("ALT_I", &pAuto->m_alt.m_I));
	CHECK_ERROR(m_pJson->getVal("ALT_IMAX", &pAuto->m_alt.m_Imax));
	CHECK_ERROR(m_pJson->getVal("ALT_D", &pAuto->m_alt.m_D));
	CHECK_ERROR(m_pJson->getVal("PWM_CENTER", (int*)&pAuto->m_alt.m_pwm));
	CHECK_ERROR(m_pJson->getVal("PWM_LOW", (int*)&pAuto->m_alt.m_pwmLow));
	CHECK_ERROR(m_pJson->getVal("PWM_HIGH", (int*)&pAuto->m_alt.m_pwmHigh));
	CHECK_ERROR(m_pJson->getVal("PWM_CENTER", (int*)&pAuto->m_alt.m_pwmCenter));
	CHECK_ERROR(m_pJson->getVal("RC_THROTTLE", (int*)&pAuto->m_alt.m_RCChannel));

	CHECK_ERROR(m_pJson->getVal("YAW_P", &pAuto->m_yaw.m_P));
	CHECK_ERROR(m_pJson->getVal("YAW_I", &pAuto->m_yaw.m_I));
	CHECK_ERROR(m_pJson->getVal("YAW_IMAX", &pAuto->m_yaw.m_Imax));
	CHECK_ERROR(m_pJson->getVal("YAW_D", &pAuto->m_yaw.m_D));
	CHECK_ERROR(m_pJson->getVal("PWM_CENTER", (int*)&pAuto->m_yaw.m_pwm));
	CHECK_ERROR(m_pJson->getVal("PWM_LOW", (int*)&pAuto->m_yaw.m_pwmLow));
	CHECK_ERROR(m_pJson->getVal("PWM_HIGH", (int*)&pAuto->m_yaw.m_pwmHigh));
	CHECK_ERROR(m_pJson->getVal("PWM_CENTER", (int*)&pAuto->m_yaw.m_pwmCenter));
	CHECK_ERROR(m_pJson->getVal("RC_YAW", (int*)&pAuto->m_yaw.m_RCChannel));


	CHECK_ERROR(m_pJson->getVal("ROLL_P", &pAuto->m_rollFar.m_P));
	CHECK_ERROR(m_pJson->getVal("ROLL_I", &pAuto->m_rollFar.m_I));
	CHECK_ERROR(m_pJson->getVal("ROLL_D", &pAuto->m_rollFar.m_D));
	CHECK_ERROR(m_pJson->getVal("Z_FAR_LIM", &pAuto->m_rollFar.m_Z));

	CHECK_ERROR(m_pJson->getVal("ROLL_P", &pAuto->m_rollNear.m_P));
	CHECK_ERROR(m_pJson->getVal("ROLL_I", &pAuto->m_rollNear.m_I));
	CHECK_ERROR(m_pJson->getVal("ROLL_D", &pAuto->m_rollNear.m_D));
	CHECK_ERROR(m_pJson->getVal("Z_NEAR_LIM", &pAuto->m_rollNear.m_Z));

	CHECK_ERROR(m_pJson->getVal("ALT_P", &pAuto->m_altFar.m_P));
	CHECK_ERROR(m_pJson->getVal("ALT_I", &pAuto->m_altFar.m_I));
	CHECK_ERROR(m_pJson->getVal("ALT_D", &pAuto->m_altFar.m_D));
	CHECK_ERROR(m_pJson->getVal("Z_FAR_LIM", &pAuto->m_altFar.m_Z));

	CHECK_ERROR(m_pJson->getVal("ALT_P", &pAuto->m_altNear.m_P));
	CHECK_ERROR(m_pJson->getVal("ALT_I", &pAuto->m_altNear.m_I));
	CHECK_ERROR(m_pJson->getVal("ALT_D", &pAuto->m_altNear.m_D));
	CHECK_ERROR(m_pJson->getVal("Z_NEAR_LIM", &pAuto->m_altNear.m_Z));

	CHECK_ERROR(m_pJson->getVal("PITCH_P", &pAuto->m_pitchFar.m_P));
	CHECK_ERROR(m_pJson->getVal("PITCH_I", &pAuto->m_pitchFar.m_I));
	CHECK_ERROR(m_pJson->getVal("PITCH_D", &pAuto->m_pitchFar.m_D));
	CHECK_ERROR(m_pJson->getVal("Z_FAR_LIM", &pAuto->m_pitchFar.m_Z));

	CHECK_ERROR(m_pJson->getVal("PITCH_P", &pAuto->m_pitchNear.m_P));
	CHECK_ERROR(m_pJson->getVal("PITCH_I", &pAuto->m_pitchNear.m_I));
	CHECK_ERROR(m_pJson->getVal("PITCH_D", &pAuto->m_pitchNear.m_D));
	CHECK_ERROR(m_pJson->getVal("Z_NEAR_LIM", &pAuto->m_pitchNear.m_Z));

	CHECK_ERROR(m_pJson->getVal("YAW_P", &pAuto->m_yawFar.m_P));
	CHECK_ERROR(m_pJson->getVal("YAW_I", &pAuto->m_yawFar.m_I));
	CHECK_ERROR(m_pJson->getVal("YAW_D", &pAuto->m_yawFar.m_D));
	CHECK_ERROR(m_pJson->getVal("Z_FAR_LIM", &pAuto->m_yawFar.m_Z));

	CHECK_ERROR(m_pJson->getVal("YAW_P", &pAuto->m_yawNear.m_P));
	CHECK_ERROR(m_pJson->getVal("YAW_I", &pAuto->m_yawNear.m_I));
	CHECK_ERROR(m_pJson->getVal("YAW_D", &pAuto->m_yawNear.m_D));
	CHECK_ERROR(m_pJson->getVal("Z_NEAR_LIM", &pAuto->m_yawNear.m_Z));


	pAuto->m_RC[pAuto->m_roll.m_RCChannel] = pAuto->m_roll.m_pwmCenter;
	pAuto->m_RC[pAuto->m_pitch.m_RCChannel] = pAuto->m_pitch.m_pwmCenter;
	pAuto->m_RC[pAuto->m_yaw.m_RCChannel] = pAuto->m_yaw.m_pwmCenter;
	pAuto->m_RC[pAuto->m_alt.m_RCChannel] = pAuto->m_alt.m_pwmCenter;


	CHECK_ERROR(m_pJson->getVal("NUM_FLOW_FRAME_RESET", &pAuto->m_resetFlowFrame));
	CHECK_ERROR(m_pJson->getVal("FLOW_RESET_FACTOR", &pAuto->m_resetFactor));
	CHECK_ERROR(m_pJson->getVal("DELAY_TIME", &pAuto->m_dT));


	CHECK_ERROR(m_pJson->getVal("TARGET_X", &pAuto->m_roll.m_targetPos));
	CHECK_ERROR(m_pJson->getVal("TARGET_Y", &pAuto->m_alt.m_targetPos));
	CHECK_ERROR(m_pJson->getVal("TARGET_Z", &pAuto->m_pitch.m_targetPos));

	return true;

}




} /* namespace kai */
