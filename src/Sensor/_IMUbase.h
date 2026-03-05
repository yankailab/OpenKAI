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
		vFloat3 m_v;
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

		void addGyro(uint64_t tStamp, const vFloat3 &vG);
		void addAcc(uint64_t tStamp, const vFloat3 &vA);
		uint64_t getIMUpair(vFloat3* pG, vFloat3* pA);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_IMUbase *)This)->update();
			return NULL;
		}

	protected:

		int m_nIMUdqMax;
		uint64_t m_tIMUpairToleranceUs;
		deque<IMU_DATA> m_dqGyro;
		deque<IMU_DATA> m_dqAcc;
	};

}
#endif
