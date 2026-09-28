/*
 * _PCmerge.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_PointCloud_PCmerge_H_
#define OpenKAI_src_Universe_Geometry_PointCloud_PCmerge_H_

#include "../_PointCloud.h"

namespace kai
{

	class _PCmerge : public _PointCloud
	{
	public:
		_PCmerge();
		virtual ~_PCmerge();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
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
		vector<PCLframe *> m_vpPCL;
		vector<uint64_t> m_vInputRevision;
		float m_rVoxel = 0.0;
	};

}
#endif
