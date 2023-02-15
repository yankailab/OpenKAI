/*
 * _RealSensePC.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_LIDAR_RealSensePC_H_
#define OpenKAI_src_LIDAR_RealSensePC_H_

#include "../3D/PointCloud/_PCframe.h"
#include <librealsense2/rs.hpp>

namespace kai
{

	class _RealSensePC : public _PCframe
	{
	public:
		_RealSensePC();
		virtual ~_RealSensePC();

		bool init(void *pKiss);
		bool open(void);
		bool start(void);
		int check(void);

	private:
		void sensorReset(void);
		bool updateRS(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_RealSensePC *)This)->update();
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
		float m_fFilterManitude;
		float m_fHolesFill;
		bool m_bAlign;
		float m_fEmitter;
		float m_fLaserPower;

		int m_rsFPS;
		int m_rsDFPS;
		bool m_bRsRGB;
		string m_vPreset;

		rs2::pointcloud m_rsPC;
		rs2::points m_rsPoints;

		bool m_bOpen;
		vInt2 m_vWHc;
		vInt2 m_vWHd;
		shared_ptr<Image> m_spImg;
		vFloat2 m_vRz; //z region
	};

}
#endif
