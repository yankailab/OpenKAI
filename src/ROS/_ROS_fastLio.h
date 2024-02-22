/*
 * _ROS_fastLio.h
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_ROS__ROS_fastLio_H_
#define OpenKAI_src_ROS__ROS_fastLio_H_

#include "_ROSbase.h"
#include "ROS_fastLio.h"

#ifdef WITH_3D
#include "../3D/PointCloud/_PCframe.h"
#endif

namespace kai
{
	class _ROS_fastLio : public _ROSbase
	{
	public:
		_ROS_fastLio();
		virtual ~_ROS_fastLio();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual int check(void);
		virtual bool start(void);
		virtual void console(void *pConsole);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ROS_fastLio *)This)->update();
			return NULL;
		}

	protected:
		shared_ptr<ROS_fastLio> m_pROSnode;

#ifdef WITH_3D
		_PCframe* m_pPCframe;
#endif

	};

}
#endif
