#ifndef OpenKAI_src_3D_PointCloud__PCgrid_H_
#define OpenKAI_src_3D_PointCloud__PCgrid_H_

#include "../_GeometryBase.h"
#include "_PCstream.h"
#include "_PCframe.h"

namespace kai
{
	struct PC_GRID_CELL
	{
		int m_nP = 0; // nPoints inside the cell
		int m_nPactivate = 1;

		void clear(void)
		{
			m_nP = 0;
		}

		void add(int n)
		{
			m_nP += n;
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

	class _PCgrid : public _GeometryBase
	{
	public:
		_PCgrid();
		virtual ~_PCgrid();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);

		// grid
		virtual bool initGrid(void);

		virtual void setPorigin(const vFloat3& vPo);
		virtual void setCellSize(const vFloat3& vCsize);
		virtual void setGridX(const vInt2& vX);
		virtual void setGridY(const vInt2& vY);
		virtual void setGridZ(const vInt2& vZ);

		virtual vFloat3 getPorigin(void);
		virtual vFloat3 getCellSize(void);
		virtual vInt2 getGridX(void);
		virtual vInt2 getGridY(void);
		virtual vInt2 getGridZ(void);

		// cell
		virtual void clearAllCells(void);
		virtual PC_GRID_CELL *getCell(const vFloat3 &vP);

		// drawing
		virtual LineSet *getGridLines(void);
		virtual void getActiveCellLines(LineSet *pLS, int cIdx);

		// data
		virtual void addPCstream(void *p, const uint64_t &tExpire);
		virtual void addPCframe(void *p);

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
		PC_GRID_CELL *m_pCell;
		int m_nCell;

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
		vFloat3 m_vAxisColX;
		vFloat3 m_vAxisColY;
		vFloat3 m_vAxisColZ;

		// active cells
		PC_GRID_ACTIVE_CELL m_pActiveCells[PCGRID_ACTIVECELL_N]; // active, alert, alarm, selected
		PC_GRID_ACTIVE_CELL *m_pCellActive;
	};

}
#endif
