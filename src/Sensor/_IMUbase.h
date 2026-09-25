/*
 * _IMUbase.h
 *
 *  Created on: March 5, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__IMUbase_H_
#define OpenKAI_src_Sensor__IMUbase_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"
#include "../Protocol/_JSONbase.h"
#include "../Dependencies/SensorFusion/SensorFusion.h"
#include <mutex>

namespace kai
{
	struct IMU_DATA
	{
		uint64_t m_t = 0; // capture timestamp in nanoseconds
		Vector3f m_v = Vector3f::Zero();
	};

	class _IMUbase : public _ModuleBase
	{
	public:
		_IMUbase();
		virtual ~_IMUbase();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual void stop(void);
		virtual bool check(void);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

		void addGyro(const Vector3f &vG, uint64_t tStamp);
		void addAcc(const Vector3f &vA, uint64_t tStamp);
		uint64_t getIMUpair(Vector3f* pG, Vector3f* pA);

	private:
		uint64_t pairFromQueues(deque<IMU_DATA> &gyro, deque<IMU_DATA> &acc, Vector3f *pG, Vector3f *pA);
		// fast thread for sensor fusion
		virtual void updateIMUfusion(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_IMUbase *)This)->update();
			return NULL;
		}

		// slower thread for data streaming
		virtual void streamIMUdata(void);
		virtual void updateStream(void);
		static void *getUpdateStream(void *This)
		{
			((_IMUbase *)This)->updateStream();
			return NULL;
		}

	protected:
		std::mutex m_mtxIMU;
		int m_nIMUdqMax = 1000;
		uint64_t m_tIMUpairToleranceNs = 5 * NSEC_MSEC; // 5ms
		deque<IMU_DATA> m_dqGyro;
		deque<IMU_DATA> m_dqAcc;
		// Preview fusion must not consume samples requested by SLAM/getIMUpair.
		deque<IMU_DATA> m_dqFusionGyro;
		deque<IMU_DATA> m_dqFusionAcc;

		// Owned exclusively by the fusion thread.
		bool m_bFusion = false;
		SF m_SF;
		uint64_t m_tStampFusion = 0;
		Quaterniond m_vOrt = Quaterniond::Identity();
		Vector3d m_vAngle = Vector3d::Zero();         // euler angles in roll, pitch, yaw order from m_vOrt

		// Published preview; the fusion writer only tries this mutex, never waits.
		std::mutex m_mtxPreview;
		Quaterniond m_vOrtPreview = Quaterniond::Identity();
		Vector3d m_vAnglePreview = Vector3d::Zero();
		uint64_t m_tFusionPreview = 0;
		bool m_bOrientationPreviewValid = false;
		bool m_bPreviewDirty = false; // Fusion-thread only.

		// data stream for preview etc.
		_Thread *m_pTstream = nullptr;
		std::mutex m_mtxStream;
		_JSONbase *m_pJb = nullptr; // Accessed only while holding m_mtxStream.
		IMU_DATA m_latestGyro, m_latestAcc; // Independent of the fusion queues.
		uint64_t m_tLastStreamG = 0, m_tLastStreamA = 0, m_tLastStreamFusion = 0;
		bool m_bOrientationValid = false;
	};

}
#endif
