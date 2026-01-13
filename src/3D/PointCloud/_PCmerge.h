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

		virtual bool init(const json &j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);

	private:
		void updateMerge(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_PCmerge *)This)->update();
			return NULL;
		}

	protected:
		vector<_GeometryBase *> m_vpGB;
		float m_rVoxel;
	};

}
#endif
