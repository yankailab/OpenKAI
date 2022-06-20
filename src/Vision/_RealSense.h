/*
 * _RealSense.h
 *
 *  Created on: Apr 6, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RealSense_H_
#define OpenKAI_src_Vision_RealSense_H_

#include "_DepthVisionBase.h"
#include "../Utility/util.h"
#include <librealsense2/rs.hpp>

namespace kai
{

	class _RealSense : public _DepthVisionBase
	{
	public:
		_RealSense();
		virtual ~_RealSense();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		bool open(void);
		void close(void);

		bool setSensorOption(const rs2::sensor& sensor, rs2_option option_type, float v);
		bool setCsensorOption(rs2_option option_type, float v);
		bool setDsensorOption(rs2_option option_type, float v);

	private:
		void sensorReset(void);
		void update(void);
		bool updateRS(void);
		static void *getUpdate(void *This)
		{
			((_RealSense *)This)->update();
			return NULL;
		}

		void updateTPP(void);
		static void *getTPP(void *This)
		{
			((_RealSense *)This)->updateTPP();
			return NULL;
		}

	public:
		string m_rsSN;
		rs2::config m_rsConfig;
		rs2::pipeline_profile m_rsProfile;
		rs2::pipeline m_rsPipe;
		rs2::frame m_rsColor;
		rs2::frame m_rsDepth;
		rs2::align *m_rspAlign;
		rs2::spatial_filter m_rsfSpat;
		rs2::decimation_filter m_rsfDec;
		bool m_bAlign;

		float m_fConfidenceThreshold;
		float m_fDigitalGain;
		float m_fPostProcessingSharpening;
		float m_fFilterManitude;
		float m_fHolesFill;
		float m_fEmitter;
		float m_fLaserPower;

		float m_fDefault;
		float m_fBrightness;
		float m_fContrast;
		float m_fGain;
		float m_fExposure;
		float m_fHue;
		float m_fSaturation;
		float m_fSharpness;
		float m_fWhiteBalance;

		int m_rsFPS;
		int m_rsDFPS;
		bool m_bRsRGB;
		string m_vPreset;

		rs2_intrinsics m_cIntrinsics;
		rs2_intrinsics m_dIntrinsics;

		//depth filter processing thread
		_Thread *m_pTPP;
	};

}
#endif
