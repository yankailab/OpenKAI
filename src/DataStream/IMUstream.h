/*
 * IMUstream.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__IMUstream__H_
#define OpenKAI_src__DataStream__IMUstream__H_

#include "DataStreamBase.h"
#include <memory>

namespace kai
{
	class IMUstream : public DataStreamBase
	{
	public:
		struct IMU_DATA
		{
			uint64_t m_t = 0; // capture timestamp in nanoseconds
			Vector3f m_v = Vector3f::Zero();
			uint64_t m_sequence = 0;
		};

		enum class Type
		{
			Gyro,
			Acc
		};

		struct Snapshot
		{
			deque<IMU_DATA> m_dqGyro;
			deque<IMU_DATA> m_dqAcc;
			uint64_t m_tStamp = 0;
			uint64_t m_revision = 0;
		};
		using SnapshotPtr = shared_ptr<const Snapshot>;

		IMUstream();
		virtual ~IMUstream();
		void console(void *pConsole) override;

		// Append a sample; retain the latest 1000 samples of each sensor type.
		void set(Type type, const Vector3f &value, uint64_t tStamp);
		// Capture bounded history on demand, keeping sensor callbacks inexpensive.
		SnapshotPtr get(void) const;

	private:
		mutable std::shared_mutex m_sMutex;
		uint64_t m_sequence = 0;
		uint64_t m_tStamp = 0;
		deque<IMU_DATA> m_dqGyro;
		deque<IMU_DATA> m_dqAcc;
	};

}
#endif
