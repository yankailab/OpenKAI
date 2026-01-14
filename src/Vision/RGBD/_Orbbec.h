/*
 * _Orbbec.h
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RGBD__Orbbec_H_
#define OpenKAI_src_Vision_RGBD__Orbbec_H_

#include <libobsensor/ObSensor.hpp>
#include "_RGBDbase.h"

namespace kai
{
	struct OrbbecCtrl
	{
		bool m_bAutoExposureToF = true;
		int m_tExposureToF = 4000;

		bool m_bAutoExposureRGB = true;
		int m_tExposureRGB = 4000;

		bool m_bFilTime = false;
		int m_filTime = 0;

		bool m_bFilConfidence = true;
		int m_filConfidence = 1;

		bool m_bFilFlyingPix = false;
		int m_filFlyingPix = 0;

		bool m_bFillHole = false;
		bool m_bSpatialFilter = false;
		bool m_bHDR = false;
	};

	class _Orbbec : public _RGBDbase
	{
	public:
		_Orbbec();
		virtual ~_Orbbec();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);

		virtual bool open(void);
		virtual void close(void);

#ifdef WITH_3D
		virtual int getPointCloud(_PCframe *pPCframe, int nPmax = INT_MAX);
#endif

	private:
		bool updateOrbbec(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Orbbec *)This)->update();
			return NULL;
		}

	private:
		OrbbecCtrl m_orbbecCtrl;

		ob::Pipeline m_obPipe;
		std::shared_ptr<ob::Config> m_spObConfig;
	};

}
#endif
