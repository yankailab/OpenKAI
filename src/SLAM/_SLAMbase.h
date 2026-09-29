/*
 * _SLAMbase.h
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__SLAMbase_H_
#define OpenKAI_src_SLAM__SLAMbase_H_

#include "../Navigation/_NavBase.h"
#include "../DataObject/IMUstream.h"
#include "../DataObject/PCLframe.h"
#include "../Protocol/_JSONbase.h"
#include <atomic>

namespace kai
{
	class _SLAMbase : public _NavBase
	{
	public:
		_SLAMbase();
		virtual ~_SLAMbase();

		bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		bool link(InstanceMgr *pM) override;
		bool start(void) override;
		void stop(void) override;
		bool check(void) override;
		void console(void *pConsole) override;

		virtual bool startTracking(void); // new session; no-op if already active
		virtual bool bTracking(void);
		virtual void stopTracking(void); // retains the last pose
		virtual void reset(void);        // stops and clears the pose and session

	protected:
		// Backend hooks run under m_mtxSLAM, including calls from the worker.
		virtual bool startSLAM(void);
		virtual void stopSLAM(void);
		virtual void resetSLAM(void);
		virtual void updateSLAM(void);

		bool readPointCloud(vector<GEOMETRY_POINT> &points, uint64_t &stamp);
		bool readIMU(Vector3d &acc, Vector3d &gyro, uint64_t &stamp);
		bool publishPose(const Isometry3d &pose, float confidence);
		// Give pending controls/status a turn between expensive SLAM frames.
		std::unique_lock<std::mutex> lockSLAM(void);

		std::mutex m_mtxSLAM;
		std::atomic_uint m_controlWaiters{0};
		PCLframe *m_pPCL = nullptr;
		uint64_t m_tPointInput = 0;
		IMUstream *m_pIMU = nullptr;
		deque<IMUstream::IMU_DATA> m_dqGyro;
		deque<IMUstream::IMU_DATA> m_dqAcc;
		bool m_bIMUbatch = false;
		uint64_t m_tStampLastGyro = 0;
		uint64_t m_tStampLastAcc = 0;
		size_t m_iGyro = 0;
		size_t m_iAcc = 0;
		uint64_t m_tStampLastFrame = 0;
		uint64_t m_tStampLastIMU = 0;
		std::atomic_bool m_bTracking{false};
		bool m_bAutoStart = true;
		string m_slamError;

	private:
		void stopTrackingLocked(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			static_cast<_SLAMbase *>(This)->update();
			return nullptr;
		}
	};
}
#endif
