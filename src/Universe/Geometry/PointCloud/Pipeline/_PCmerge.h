/*
 * _PCmerge.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_PointCloud_PCmerge_H_
#define OpenKAI_src_Universe_Geometry_PointCloud_PCmerge_H_

#include "../../../_ReferenceFrame.h"
#include "../../../../DataObject/PCLframe.h"

namespace kai
{

	class _PCmerge : public _ReferenceFrame
	{
	public:
		_PCmerge();
		virtual ~_PCmerge();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);
		virtual bool check(void);
		virtual void clear(void);

	private:
		void updateMerge(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_PCmerge *)This)->update();
			return NULL;
		}

	protected:
		PCLframe *m_pPCLout = nullptr;
		vector<PCLframe *> m_vpPCLin;
		vector<uint64_t> m_vInputStamps;
		vector<vector<GEOMETRY_POINT>> m_vInputPoints;
		float m_rVoxel = 0.0;
	};

}
#endif
