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

namespace kai
{
	struct IMU_DATA
	{
		uint64_t m_t;
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
		virtual bool check(void);
		virtual void console(void *pConsole);

		void addGyro(uint64_t tStamp, const Vector3f &vG);
		void addAcc(uint64_t tStamp, const Vector3f &vA);
		uint64_t getIMUpair(Vector3f* pG, Vector3f* pA);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_IMUbase *)This)->update();
			return NULL;
		}

	protected:

		int m_nIMUdqMax = 1000;
		uint64_t m_tIMUpairToleranceUs = 5000; // 5ms
		deque<IMU_DATA> m_dqGyro;
		deque<IMU_DATA> m_dqAcc;
	};

}
#endif
