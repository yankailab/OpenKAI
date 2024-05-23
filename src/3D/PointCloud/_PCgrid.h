#ifndef OpenKAI_src_3D_PointCloud__PCgrid_H_
#define OpenKAI_src_3D_PointCloud__PCgrid_H_

#include "../_GeometryBase.h"
#include "_PCstream.h"
#include "_PCframe.h"

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
		virtual bool link(void);
        virtual bool start(void);
		virtual int check(void);

		virtual bool initBuffer(void);
		virtual void clear(void);
		virtual PC_GRID_CELL *getCell(const vFloat3 &vP);
		virtual LineSet *getGridLines(void);
		virtual LineSet *getActiveCellLines(void);

		virtual void getPCstream(void *p, const uint64_t &tExpire);
		virtual void getPCframe(void *p);

	protected:
		virtual PC_GRID_CELL *getCell(const vInt3 &vPi);

		virtual void generateGridLines(void);
		void addGridAxisLine(int nDa,
							 const vFloat2 &vRa,
							 float csA,
							 int nDb,
							 const vFloat2 &vRb,
							 float csB,
							 const vFloat2 &vRc,
							 const vInt3 &vAxis,
							 const vFloat3 &vCol);

		void updateActiveCell(void);
		void updateActiveCellLines(void);
		void generateActiveCellLines(void);
		void addActiveCellLine(const vFloat3 &pA,
							   const vFloat3 &pB,
							   const vFloat3 &vCol);

	private:
		void updatePCgrid(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_PCgrid *)This)->update();
			return NULL;
		}

	protected:
		PC_GRID_CELL *m_pCell;
		int m_nCell;

		int m_nPcellThr;

		// point cloud input
		vector<_GeometryBase *> m_vpGB;
		uint64_t m_tExpire;

		// cell number of each axis
		vInt3 m_vDim;
		int m_dYZ;

		// grid positions and regions
		vFloat3 m_vPmin;
		vFloat2 m_vRx;
		vFloat2 m_vRy;
		vFloat2 m_vRz;
		vFloat3 m_vOvCellSize;

		// grid generating params
		vFloat3 m_vPorigin;

		// cell number around the origin for each direction of each axis
		vInt2 m_vX;
		vInt2 m_vY;
		vInt2 m_vZ;

		// dimension for each cell
		vFloat3 m_vCellSize;

		// visualization
		bool m_bVisual;

		// static grid
		LineSet m_gridLine;
		int m_gLineWidth;
		vFloat3 m_vAxisColX;
		vFloat3 m_vAxisColY;
		vFloat3 m_vAxisColZ;

		// active cells
		LineSet m_activeCellLine; // lines for active cell
		int m_activeCellLineWidth;
		vFloat3 m_vActiveCellLineCol;
		vector<vInt3> m_vActiveCellIdx;

		tSwap<vector<vInt3>> m_svActiveCellIdx;
		tSwap<LineSet> m_sActiveCellLine;
	};

}
#endif
