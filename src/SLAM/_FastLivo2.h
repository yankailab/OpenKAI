/*
 * _FastLivo2.h
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__FastLivo2_H_
#define OpenKAI_src_SLAM__FastLivo2_H_

#include "../Vision/_VisionBase.h"
#include "_SLAMbase.h"

#include <array>
#include <chrono>
#include <fastlivo2_core/FastLivo2Core.hpp>

using fastlivo2_core::FastLivo2Core;
using fastlivo2_core::ImageFrame;
using fastlivo2_core::ImuSample;
using fastlivo2_core::LidarPoint;
using fastlivo2_core::LidarScan;

namespace kai
{
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

		ImageFrame makeTestBGR(double t, int w, int h);
		LidarScan makeTestScan(double t0, double t1, int npts);

	private:
		void updateFastLivo2(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_FastLivo2 *)This)->update();
			return NULL;
		}

	protected:
		FastLivo2Core core;

		// synthetic demo
		double t;
		double T_end;

		double imu_dt;
		double scan_dt;
		double img_dt;

		double next_scan;
		double next_img;

		int W;
		int H;

	};

}
#endif
