#ifndef OpenKAI_src_3D_Grid__OctreeBase_H_
#define OpenKAI_src_3D_Grid__OctreeBase_H_

#include "../../Filter/Median.h"
#include "../_GeometryBase.h"
#include "../PointCloud/_PointCloud.h"

namespace kai
{

	class _OctreeBase : public _GeometryBase
	{
	public:
		_OctreeBase();
		virtual ~_OctreeBase();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);

		// config
		virtual bool loadConfig(json *pJ = nullptr, string fName = "");
		virtual bool saveConfig(json &j, string fName = "");

		// grid
		virtual bool initGeometry(void);

		// cell

		// drawing

		// data
		virtual void addPointCloud(void *p, const uint64_t tExpire = 0);

		// save/load

	protected:
		virtual void updateOctree(void);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_OctreeBase *)This)->update();
			return NULL;
		}

	protected:
		// grid generating params
		vDouble3 m_vPorigin;


		// point cloud input
		vector<_GeometryBase *> m_vpGb;
		uint64_t m_dTexpire;

	};

}
#endif
