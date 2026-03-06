/*
 * _FastLivo2.h
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__FastLivo2_H_
#define OpenKAI_src_SLAM__FastLivo2_H_

#include "../3D/PointCloud/_PCstream.h"
#include "_SLAMbase.h"
#include <array>
#include <chrono>
#include <fastlivo2_core/FastLivo2Core.hpp>

using namespace fastlivo2_core;

namespace kai
{
	struct FASTLIVO2_CONFIG
	{
		int m_nFdownSample = 20;
		size_t m_nPmax = 300000;
		double m_voxelLeafSize = 0.1;

	};

	class _FastLivo2 : public _SLAMbase
	{
	public:
		_FastLivo2();
		virtual ~_FastLivo2();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

		void reset(void);

		ImageFrame makeTestBGR(double t, int w, int h);
		LidarScan makeTestScan(double t0, double t1, int npts);

	private:
		bool setup(void);
		void updatePointCloud(void);
		bool updateFastLivo2(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_FastLivo2 *)This)->update();
			return NULL;
		}

	protected:
		_PCstream* m_pPCin;
		_PCstream* m_pPCout;
		int m_iP;
		int m_nPmax;
		uint64_t m_tStampP;

		// core
		FastLivo2Core m_fastLivo;

		// config
		FASTLIVO2_CONFIG m_config;

		// point cloud
		// extrinsics, IMU<-LiDAR: pImu = ILr * pLidar + ILt
		array<double, 9> m_aILr = {1, 0, 0, 0, 1, 0, 0, 0, 1};
		array<double, 3> m_aILt = {0, 0, 0};

		// img
		bool m_bImg;
		vInt2 m_vSizeImg;
		vDouble2 m_vF; // focal
		vDouble2 m_vC; // center
		array<double, 5> m_aDist  = {0.0, 0.0, 0.0, 0.0, 0.0}; // k1,k2,p1,p2,k3
		// extrinsics, Camera<-LiDAR: pCam = CLr * pLidar + CLt
		array<double, 9> m_aCLr = {1, 0, 0, 0, 1, 0, 0, 0, 1};
		array<double, 3> m_aCLt = {0.05, 0.0, 0.0}; // example 5c

	};

}
#endif
