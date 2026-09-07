#ifndef OpenKAI_src_3D_Grid__OctreeBase_H_
#define OpenKAI_src_3D_Grid__OctreeBase_H_

#include "../../Filter/Median.h"
#include "../_GeometryBase.h"
#include "../PointCloud/_PointCloud.h"

#define N_OCT 8

namespace kai
{
	union uint128
	{
		uint8_t m_uint8[16];
		uint16_t m_uint16[8];
		uint32_t m_uint32[4];
		uint64_t m_uint64[2];
	};

	struct OCTREE_CELL
	{
		uint128 m_UGLID;
		uint8_t m_Lidx;

		OCTREE_CELL *m_pParent;
		OCTREE_CELL *m_pChild[N_OCT];
		uint8_t m_nChild;

		bool addChild(int Lidx)
		{
		}

		OCTREE_CELL *getChild(int Lidx)
		{
		}

		int getLevel(void)
		{
		}

		void release(void)
		{
		}
	};

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
		// grid generating
		vDouble3 m_vPorigin;

		// point cloud input
		vector<_GeometryBase *> m_vpGb;
		uint64_t m_dTexpire;
	};

}
#endif
