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

		enum class Type
		{
			Gyro,
			Acc
		};

		IMUstream();
		virtual ~IMUstream();
		void console(void *pConsole) override;

		// Append a sample; retain the latest 1000 samples of each sensor type.
		void set(Type type, const Vector3f &value, uint64_t tStamp);
		uint64_t get(deque<IMU_DATA> &gyro, deque<IMU_DATA> &acc);

	private:
		std::shared_mutex m_sMutex;
		deque<IMU_DATA> m_dqGyro;
		deque<IMU_DATA> m_dqAcc;
	};

}
#endif
