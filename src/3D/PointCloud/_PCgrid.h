#ifndef OpenKAI_src_3D_PointCloud__PCgrid_H_
#define OpenKAI_src_3D_PointCloud__PCgrid_H_

#include "../_GeometryBase.h"
#include "../../Filter/Median.h"
#include "_PCstream.h"
#include "_PCframe.h"

namespace kai
{
	struct PC_GRID_CELL
	{
		Median<int> m_med;
		int m_nPraw = 0; // raw nPoints inside the cell
		int m_nP = 0; // filtered
		int m_nPactivate = 1;

		void init(int nW = 3)
		{
			m_med.init(nW);
			clear();
		}

		void clear(void)
		{
			m_nPraw = 0;
			m_nP = 0;
		}

		void add(int n)
		{
			m_nPraw += n;
		}

		void updateFilter(void)
		{
			m_nP = m_med.update(m_nPraw);
		}

		int nP(void)
		{
			return m_nP;
		}

		int nPraw(void)
		{
			return m_nPraw;
		}
	};

	struct PC_GRID_ACTIVE_CELL
	{
		vector<vInt3> m_vCellIdx;
		LineSet m_LS;

		bool empty(void)
		{
			return m_vCellIdx.empty();
		}

		void clearCells(void)
		{
			m_vCellIdx.clear();
		}

		void clearLS(void)
		{
			m_LS.Clear();
			m_LS.points_.clear();
			m_LS.lines_.clear();
			m_LS.colors_.clear();
		}

		int getCellIdx(const vInt3 &vCidx)
		{
			for (int i = 0; i < m_vCellIdx.size(); i++)
			{
				vInt3 *pI = &m_vCellIdx[i];
				if (*pI == vCidx)
					return i;
			}

			return -1;
		}

		void deleteCell(const vInt3 &vCidx)
		{
			int i = getCellIdx(vCidx);
			IF_(i < 0);

			vector<vInt3>::iterator it = m_vCellIdx.begin() + i;
			m_vCellIdx.erase(it);
		}

		bool addCell(const vInt3 &vCidx)
		{
			IF_F(getCellIdx(vCidx) >= 0);	// already existed

			m_vCellIdx.push_back(vCidx);
			return true;
		}

		void addCellLine(const vFloat3 &pA,
						 const vFloat3 &pB)
		{
			int nP = m_LS.points_.size();
			m_LS.points_.push_back(Vector3d{pA.x, pA.y, pA.z});
			m_LS.points_.push_back(Vector3d{pB.x, pB.y, pB.z});
			m_LS.lines_.push_back(Vector2i{nP, nP + 1});
		}

		void generateLS(const vFloat2 &vRx,
						const vFloat2 &vRy,
						const vFloat2 &vRz,
						const vFloat3 &vCsize)
		{
			vFloat3 pO, pA, pB;

			for (vInt3 vC : m_vCellIdx)
			{
				pO.x = vRx.x + vCsize.x * vC.x;
				pO.y = vRy.x + vCsize.y * vC.y;
				pO.z = vRz.x + vCsize.z * vC.z;

				pA = pO;
				pB = pA;
				pB.x += vCsize.x;
				addCellLine(pA, pB);

				pB = pA;
				pB.y += vCsize.y;
				addCellLine(pA, pB);

				pB = pA;
				pB.z += vCsize.z;
				addCellLine(pA, pB);

				pA = pO;
				pA.x += vCsize.x;
				pA.y += vCsize.y;

				pB = pA;
				pB.x -= vCsize.x;
				addCellLine(pA, pB);

				pB = pA;
				pB.y -= vCsize.y;
				addCellLine(pA, pB);

				pB = pA;
				pB.z += vCsize.z;
				addCellLine(pA, pB);

				pA = pO;
				pA.x += vCsize.x;
				pA.z += vCsize.z;

				pB = pA;
				pB.x -= vCsize.x;
				addCellLine(pA, pB);

				pB = pA;
				pB.y += vCsize.y;
				addCellLine(pA, pB);

				pB = pA;
				pB.z -= vCsize.z;
				addCellLine(pA, pB);

				pA = pO;
				pA.y += vCsize.y;
				pA.z += vCsize.z;

				pB = pA;
				pB.x += vCsize.x;
				addCellLine(pA, pB);

				pB = pA;
				pB.y -= vCsize.y;
				addCellLine(pA, pB);

				pB = pA;
				pB.z -= vCsize.z;
				addCellLine(pA, pB);
			}
		}
	};

#define PCGRID_ACTIVECELL_N 4
#define PC_GRID_N_CELL 1000000

	class _PCgrid : public _GeometryBase
	{
	public:
		_PCgrid();
		virtual ~_PCgrid();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);

		// grid
		virtual int initGeometry(void);

		virtual void setPorigin(const vFloat3& vPo);
		virtual void setCellSize(const vFloat3& vCsize);
		virtual void setCellRangeX(const vInt2& vCrX);
		virtual void setCellRangeY(const vInt2& vCrY);
		virtual void setCellRangeZ(const vInt2& vCrZ);

		virtual vFloat3 getPorigin(void);
		virtual vFloat3 getCellSize(void);
		virtual vInt2 getCellRangeX(void);
		virtual vInt2 getCellRangeY(void);
		virtual vInt2 getCellRangeZ(void);

		// cell
		virtual void clearAllCells(void);
		virtual PC_GRID_CELL *getCell(const vFloat3 &vP);

		// drawing
		virtual LineSet *getGridLines(void);
		virtual void getActiveCellLines(LineSet *pLS, int cIdx);

		// data
		virtual void addPCstream(void *p, const uint64_t &tExpire);
		virtual void addPCframe(void *p);

		// save/load

	protected:
		PC_GRID_CELL *getCell(const vInt3 &vC);
		int getCell1Didx(const vInt3 &vC);

		void generateGridLines(void);
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
		void updateActiveCellLS(PC_GRID_ACTIVE_CELL* pAcell);

		virtual void updatePCgrid(void);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_PCgrid *)This)->update();
			return NULL;
		}

	protected:
		PC_GRID_CELL m_pCell[PC_GRID_N_CELL];
		int m_nCell;
		int m_nMedWidth;

		// point cloud input
		vector<_GeometryBase *> m_vpGB;
		uint64_t m_tExpire;

		//Grid config
		// grid generating params
		vFloat3 m_vPorigin;

		// cell number around the origin for each direction of each axis
		vInt2 m_vCellRangeX;
		vInt2 m_vCellRangeY;
		vInt2 m_vCellRangeZ;

		// dimension for each cell
		vFloat3 m_vCellSize;

		// Grid param
		// cell number of each axis
		vInt3 m_vDim;
		int m_dYZ;

		// grid positions and regions
		vFloat3 m_vPmin;
		vFloat2 m_vRx;
		vFloat2 m_vRy;
		vFloat2 m_vRz;
		vFloat3 m_vOvCellSize;

		// visualization
		bool m_bVisual;

		// static grid
		LineSet m_gridLine;
		vFloat3 m_vAxisColX;
		vFloat3 m_vAxisColY;
		vFloat3 m_vAxisColZ;

		// active cells
		PC_GRID_ACTIVE_CELL m_pActiveCells[PCGRID_ACTIVECELL_N]; // active, alert, alarm, selected
		PC_GRID_ACTIVE_CELL *m_pCellActive;
	};

}
#endif
