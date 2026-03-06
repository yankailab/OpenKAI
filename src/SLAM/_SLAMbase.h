/*
 * _SLAMbase.h
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__SLAMbase_H_
#define OpenKAI_src_SLAM__SLAMbase_H_

#include "../Navigation/_NavBase.h"
#include "../Sensor/_IMUbase.h"

namespace kai
{

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

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_SLAMbase *)This)->update();
			return NULL;
		}

	protected:
		_IMUbase* m_pIMU;
		double m_tScaleIMU;
		double m_tScalePC;

		bool m_bTracking;

	};

}
#endif
