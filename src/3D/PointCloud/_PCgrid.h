#ifndef OpenKAI_src_3D_PointCloud__PCgrid_H_
#define OpenKAI_src_3D_PointCloud__PCgrid_H_

#include "../_GeometryBase.h"

namespace kai
{
	struct PC_GRID_CELL
	{
		uint32_t m_nP; // nPoints inside the cell

		void clear(void)
		{
			m_nP = 0;
		}

		void add(int n)
		{
			m_nP += n;
		}
	};

	class _PCgrid : public _GeometryBase
	{
	public:
		_PCgrid();
		virtual ~_PCgrid();

		virtual bool init(void *pKiss);
		virtual int check(void);

		virtual bool initBuffer(void);
        virtual void clear(void);
        virtual void getStream(void* p, const uint64_t& tExpire);

		virtual PC_GRID_CELL* getCell(const vFloat3& vP);

	protected:
		virtual PC_GRID_CELL* getCell(const vInt3& vPi);

	protected:
		PC_GRID_CELL *m_pCell;
		int m_nCell;
		vInt3 m_vDim;
		int m_dYZ;
		vFloat3 m_vPmin;
		vFloat2 m_vRx;
		vFloat2 m_vRy;
		vFloat2 m_vRz;
		vFloat3 m_vOvCellSize;

		// grid generating params
		vFloat3 m_vPorigin;
		vInt2 m_vX;
		vInt2 m_vY;
		vInt2 m_vZ;
		vFloat3 m_vCellSize;
	};

}
#endif
