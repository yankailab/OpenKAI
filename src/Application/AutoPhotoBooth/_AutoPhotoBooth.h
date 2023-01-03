/*
 * _AutoPhotoBooth.h
 *
 *  Created on: Dec 24, 2022
 *      Author: yankai
 */

#ifndef OpenKAI_src_RobotArm__AutoPhotoBooth_H_
#define OpenKAI_src_RobotArm__AutoPhotoBooth_H_

#include "../../Base/common.h"
#include "../../Actuator/_ActuatorBase.h"

namespace kai
{

	class _AutoPhotoBooth : public _ModuleBase
	{
	public:
		_AutoPhotoBooth(void);
		virtual ~_AutoPhotoBooth();

		bool init(void *pKiss);
		bool start(void);
		int check(void);

		void on(void);
		void off(void);
		void updateAction(void);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_AutoPhotoBooth *)This)->update();
			return NULL;
		}

	public:
		bool m_bON;
		int m_iAction;
		bool m_bComplete;
	};

}
#endif
