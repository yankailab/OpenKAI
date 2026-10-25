/*
 * _PCremove.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_PointCloud_PCremove_H_
#define OpenKAI_src_Universe_Geometry_PointCloud_PCremove_H_

#include "../../../_TransformRigid.h"

namespace kai
{

	class _PCremove : public _ModuleBase
	{
	public:
		_PCremove();
		virtual ~_PCremove();

		virtual bool loadConfig(void) override;
		bool link(InstanceMgr *pM) override;
		bool saveConfig(bool bExport) override;
		virtual bool start(void);
		virtual bool check(void);

	private:
		virtual void update(void);
		void updateFilter(void);
		static void *getUpdate(void *This)
		{
			((_PCremove *)This)->update();
			return NULL;
		}

	protected:
		_TransformRigid *m_pTf = nullptr;
		int m_nP = 16;
		double m_r = 0.05;
	};

}
#endif
