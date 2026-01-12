/*
 * _RGBD2PCframe.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud__RGBD2PCframe_H_
#define OpenKAI_src_3D_PointCloud__RGBD2PCframe_H_

#include "_PCframe.h"
#include "../../Vision/RGBD/_RGBDbase.h"

namespace kai
{

	class _RGBD2PCframe : public _PCframe
	{
	public:
		_RGBD2PCframe();
		virtual ~_RGBD2PCframe();

		virtual bool init(const json &j);
		virtual bool link(const json& j, ModuleMgr* pM);


		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);

	private:
		void getPointCloud(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_RGBD2PCframe *)This)->update();
			return NULL;
		}

	protected:
		_RGBDbase *m_pV;
	};

}
#endif
