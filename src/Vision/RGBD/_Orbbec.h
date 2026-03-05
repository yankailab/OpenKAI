/*
 * _Orbbec.h
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RGBD__Orbbec_H_
#define OpenKAI_src_Vision_RGBD__Orbbec_H_

#include <libobsensor/ObSensor.hpp>
#include <libobsensor/h/ObTypes.h>
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
		virtual void console(void *pConsole);

		virtual bool open(void);
		virtual void close(void);

	private:
		bool updateOrbbec(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Orbbec *)This)->update();
			return NULL;
		}

#ifdef WITH_3D
		void updatePC(void);
#endif
		void updateDepth(void);
		void updateTPP(void);
		static void *getTPP(void *This)
		{
			((_Orbbec *)This)->updateTPP();
			return NULL;
		}

		uint64_t frameTsUs_(const shared_ptr<ob::Frame> &f)
		{
			// Prefer "system timestamp us" if available in your SDK build.
			// If not, fall back to other timestamp APIs.
			return f->getSystemTimeStampUs();
		}

	protected:
		string m_SN;
		ob::Context m_ctx;
		shared_ptr<ob::Device> m_spDev;
		shared_ptr<ob::Pipeline> m_spPipe;
		shared_ptr<ob::SensorList> m_spSensorList;
		shared_ptr<ob::Config> m_spConfig;
		uint32_t m_tOutMs;

		shared_ptr<ob::PointCloudFilter> m_spPCF;
		shared_ptr<ob::Frame> m_spFrame;

		uint64_t m_tDus;
		uint64_t m_dtDus;
		uint64_t m_tRGBus;
		uint64_t m_dtRGBus;

		OrbbecCtrl m_orbbecCtrl;
	};

}
#endif
