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
	static inline double us_to_s(uint64_t us) { return double(us) * 1e-6; }

	struct TimedVec3
	{
		uint64_t t_us;
		Eigen::Vector3d v;
	};

	class ImuPairer
	{
	public:
		void pushGyro(uint64_t t_us, const Eigen::Vector3d &g)
		{
			m_dqGyro.push_back({t_us, g});
		}

		void pushAcc(uint64_t t_us, const Eigen::Vector3d &a)
		{
			m_dqAcc.push_back({t_us, a});
		}

		TimedVec3 getGyro(void) const
		{
			if (m_dqGyro.empty())
				return {0, {0, 0, 0}};

			return m_dqGyro.back();
		}

		TimedVec3 getAcc(void) const
		{
			if (m_dqAcc.empty())
				return {0, {0, 0, 0}};

			return m_dqAcc.back();
		}

		// Emit as many paired samples as possible
		// void flushToCore(FastLivo2Core &core)
		// {
		// 	const uint64_t MAX_DT = 5000; // 5ms pairing tolerance
		// 	while (!m_dqGyro.empty() && !m_dqAcc.empty())
		// 	{
		// 		auto tg = m_dqGyro.front().t_us;
		// 		auto ta = m_dqAcc.front().t_us;

		// 		if (tg + MAX_DT < ta)
		// 		{
		// 			m_dqGyro.pop_front();
		// 			continue;
		// 		}
		// 		if (ta + MAX_DT < tg)
		// 		{
		// 			m_dqAcc.pop_front();
		// 			continue;
		// 		}

		// 		ImuSample s;
		// 		s.t = us_to_s(std::max(tg, ta));
		// 		s.gyro = m_dqGyro.front().v;
		// 		s.acc = m_dqAcc.front().v;
		// 		core.pushImu(s);

		// 		m_dqGyro.pop_front();
		// 		m_dqAcc.pop_front();
		// 	}

		// 	// (Optional) prune if queues grow too big
		// 	while (m_dqGyro.size() > 2000)
		// 		m_dqGyro.pop_front();
		// 	while (m_dqAcc.size() > 2000)
		// 		m_dqAcc.pop_front();
		// }

	private:
		deque<TimedVec3> m_dqGyro;
		deque<TimedVec3> m_dqAcc;
	};

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

		void updateTPP(void);
		static void *getTPP(void *This)
		{
			((_Orbbec *)This)->updateTPP();
			return NULL;
		}

		uint64_t frameTsUs_(const std::shared_ptr<ob::Frame> &f)
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
		ImuPairer m_IMUpair;

		OrbbecCtrl m_orbbecCtrl;
	};

}
#endif
