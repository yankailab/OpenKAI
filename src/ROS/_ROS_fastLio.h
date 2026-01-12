/*
 * _ROS_fastLio.h
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_ROS__ROS_fastLio_H_
#define OpenKAI_src_ROS__ROS_fastLio_H_

#include "../Navigation/_NavBase.h"
#ifdef WITH_3D
#include "../3D/PointCloud/_PCframe.h"
#endif

#include "ROS_fastLio.h"

namespace kai
{
	class _ROS_fastLio : public _NavBase
	{
	public:
		_ROS_fastLio();
		virtual ~_ROS_fastLio();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool check(void);
		virtual bool start(void);
		virtual void console(void *pConsole);

	private:
		void updateNav(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ROS_fastLio *)This)->update();
			return NULL;
		}

		void updateROS(void);
		static void *getUpdateROS(void *This)
		{
			((_ROS_fastLio *)This)->updateROS();
			return NULL;
		}

	protected:
		_Thread *m_pTros;

		shared_ptr<ROS_fastLio> m_pROSnode;

#ifdef WITH_3D
		_PCframe* m_pPCframe;
#endif

	};

}
#endif
