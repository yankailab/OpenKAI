/*
 * _PCmerge.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud_PCmerge_H_
#define OpenKAI_src_3D_PointCloud_PCmerge_H_

#include "../../Base/common.h"
#include "../_GeometryBase.h"

namespace kai
{

	class _PCmerge : public _GeometryBase
	{
	public:
		_PCmerge();
		virtual ~_PCmerge();

		int init(void *pKiss);
		int start(void);
		int check(void);

	private:
		void updateMerge(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_PCmerge *)This)->update();
			return NULL;
		}

	public:
		vector<_GeometryBase *> m_vpGB;
		float m_rVoxel;
	};

}
#endif
