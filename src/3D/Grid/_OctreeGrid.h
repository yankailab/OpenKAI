#ifndef OpenKAI_src_3D_Grid__OctreeGrid_H_
#define OpenKAI_src_3D_Grid__OctreeGrid_H_

#include "_OctreeBase.h"
#include "../PointCloud/_PointCloud.h"
#include "../../Filter/Median.h"

namespace kai
{
	class _OctreeGrid : public _OctreeBase
	{
	public:
		_OctreeGrid();
		virtual ~_OctreeGrid();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);

		// config
		virtual bool loadConfig(json *pJ = nullptr, string fName = "");
		virtual bool saveConfig(json &j, string fName = "");

		// data
		virtual void addPointCloud(void *p, const uint64_t tExpire = 0);

		// drawing
		virtual int get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pGrPout, uint64_t tExpire = 0);
		virtual int get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pGrLOut, uint64_t tExpire = 0);

	protected:
		virtual void updateOctreeGrid(void);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_OctreeGrid *)This)->update();
			return NULL;
		}

	protected:
		vDouble3 m_vPorigin;
		vFloat3 m_vRootCellSize; // root layer cell size in meters

		// point cloud input
		vector<_GeometryBase *> m_vpGb;
		uint64_t m_dTexpire;
	};

}
#endif
