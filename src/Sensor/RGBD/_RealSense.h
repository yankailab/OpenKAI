/*
 * _RealSense.h
 *
 *  Created on: Apr 6, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_RGBD_RealSense_H_
#define OpenKAI_src_RGBD_RealSense_H_

#include "_RGBDbase.h"
#include <librealsense2/rs.hpp>

namespace kai
{

	class _RealSense : public _RGBDbase
	{
	public:
		_RealSense();
		virtual ~_RealSense();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);

		virtual bool open(void);
		virtual void close(void);

		bool setSensorOption(const rs2::sensor& sensor, rs2_option option_type, float v);
		bool setCsensorOption(rs2_option option_type, float v);
		bool setDsensorOption(rs2_option option_type, float v);

		bool getSensorOption(const rs2::sensor& sensor, rs2_option option_type, rs2::option_range* pR);
		bool getCsensorOption(rs2_option option_type, rs2::option_range* pR);
		bool getDsensorOption(rs2_option option_type, rs2::option_range* pR);

	protected:
		void sensorReset(void);
		bool updatePointCloud(void);
		bool updateRS(void);
		void update(void);
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
		float m_fFilterMagnitude;
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

		float m_dScale;

		// point cloud
		rs2::pointcloud m_rsPC;
		rs2::points m_rsPoints;
		vInt2 m_vWHc;
		vInt2 m_vWHd;
//		shared_ptr<Image> m_spImg;
		vFloat2 m_vRz; //z region


		//depth filter processing thread
		_Thread *m_pTPP;
	};

}
#endif
