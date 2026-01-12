/*
 * _RealSense.h
 *
 *  Created on: Apr 6, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RGBD__RealSense_H_
#define OpenKAI_src_Vision_RGBD__RealSense_H_

#include "_RGBDbase.h"
#include <librealsense2/rs.hpp>

namespace kai
{
	struct RS_CTRL
	{
		float m_fDefault;

		float m_fConfidenceThreshold;
		float m_fDigitalGain;
		float m_fPostProcessingSharpening;
		float m_fFilterMagnitude;
		float m_fHolesFill;
		float m_fEmitter;
		float m_fLaserPower;

		float m_fBrightness;
		float m_fContrast;
		float m_fGain;
		float m_fExposure;
		float m_fHue;
		float m_fSaturation;
		float m_fSharpness;
		float m_fWhiteBalance;

		void clear(void)
		{
			m_fDefault = 10e6;

			m_fConfidenceThreshold = m_fDefault;
			m_fDigitalGain = m_fDefault;
			m_fPostProcessingSharpening = m_fDefault;
			m_fFilterMagnitude = m_fDefault;
			m_fHolesFill = m_fDefault;
			m_fEmitter = m_fDefault;
			m_fLaserPower = m_fDefault;

			m_fBrightness = m_fDefault;
			m_fContrast = m_fDefault;
			m_fGain = m_fDefault;
			m_fExposure = m_fDefault;
			m_fHue = m_fDefault;
			m_fSaturation = m_fDefault;
			m_fSharpness = m_fDefault;
			m_fWhiteBalance = m_fDefault;
		}
	};

	class _RealSense : public _RGBDbase
	{
	public:
		_RealSense();
		virtual ~_RealSense();

		virtual bool init(const json& j);
		virtual bool start(void);
		virtual bool check(void);

		virtual bool open(void);
		virtual void close(void);

#ifdef WITH_3D
		virtual int getPointCloud(_PCframe* pPCframe, int nPmax = INT_MAX);
#endif

		bool setSensorOption(const rs2::sensor &sensor, rs2_option option_type, float v);
		bool setCsensorOption(rs2_option option_type, float v);
		bool setDsensorOption(rs2_option option_type, float v);

		bool getSensorOption(const rs2::sensor &sensor, rs2_option option_type, rs2::option_range *pR);
		bool getCsensorOption(rs2_option option_type, rs2::option_range *pR);
		bool getDsensorOption(rs2_option option_type, rs2::option_range *pR);

	private:
		void sensorReset(void);
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

	protected:
		string m_rsSN;
		rs2::config m_rsConfig;
		rs2::pipeline_profile m_rsProfile;
		rs2::pipeline m_rsPipe;
		rs2::frame m_rsColor;
		rs2::frame m_rsDepth;
		rs2::align *m_rspAlign;
		rs2::spatial_filter m_rsfSpat;
		rs2::decimation_filter m_rsfDec;
		RS_CTRL m_rsCtrl;

		int m_rsFPS;
		int m_rsDFPS;
		bool m_bAlign;
		string m_vPreset;

		rs2_intrinsics m_cIntrinsics;
		rs2_intrinsics m_dIntrinsics;

		// point cloud
		rs2::pointcloud m_rsPC;
		rs2::points m_rsPoints;
		//		shared_ptr<Image> m_spImg;
	};

}
#endif
