/*
 * IMUstream.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__IMUstream__H_
#define OpenKAI_src__DataStream__IMUstream__H_

#include "DataObjBase.h"

namespace kai
{
	class IMUstream : public DataObjBase
	{
	public:
		struct IMU_DATA
		{
			uint64_t m_t = 0; // capture timestamp in nanoseconds
			Vector3f m_v = Vector3f::Zero();
		};

		IMUstream();
		virtual ~IMUstream();
		void console(void *pConsole) override;

		bool loadConfig(void);
		bool saveConfig(bool bExport = false);

		void addGyro(const Vector3f &value, uint64_t tStamp);
		void addAcc(const Vector3f &value, uint64_t tStamp);
		uint64_t get(deque<IMU_DATA> &gyro, deque<IMU_DATA> &acc);

	private:
		int m_nBuf = 1000;

		deque<IMU_DATA> m_dqGyro;
		deque<IMU_DATA> m_dqAcc;
		std::shared_mutex m_sMutex;
	};

}
#endif
