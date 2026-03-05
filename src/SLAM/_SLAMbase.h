/*
 * _SLAMbase.h
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__SLAMbase_H_
#define OpenKAI_src_SLAM__SLAMbase_H_

#include "../Navigation/_NavBase.h"

namespace kai
{
	static inline double us_to_s(uint64_t us) { return double(us) * 1e-6; }

	struct TimedVec3
	{
		uint64_t t_us;
		Eigen::Vector3d v;
	};

	class _SLAMbase : public _NavBase
	{
	public:
		_SLAMbase();
		virtual ~_SLAMbase();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

		bool bTracking(void);

		void pushGyro(uint64_t t_us, const Eigen::Vector3d &g);
		void pushAcc(uint64_t t_us, const Eigen::Vector3d &a);
		bool getIMUpair(Vector3d* pGyro, Vector3d* pAcc);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_SLAMbase *)This)->update();
			return NULL;
		}

	protected:
		bool m_bTracking;

		int m_nIMUdqMax;
		deque<TimedVec3> m_dqGyro;
		deque<TimedVec3> m_dqAcc;
	};

}
#endif
