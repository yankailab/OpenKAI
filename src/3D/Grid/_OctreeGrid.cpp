#include "_OctreeGrid.h"

namespace kai
{
	namespace
	{
		static vFloat3 vHalf(const vFloat3 &v)
		{
			return vFloat3(v.x * 0.5f, v.y * 0.5f, v.z * 0.5f);
		}

		static vFloat3 childCenter(const vFloat3 &vPc, const vFloat3 &vSize, uint8_t iC)
		{
			float kX = (iC & 4) ? 0.25f : -0.25f;
			float kY = (iC & 2) ? 0.25f : -0.25f;
			float kZ = (iC & 1) ? 0.25f : -0.25f;

			return vFloat3(vPc.x + vSize.x * kX,
						   vPc.y + vSize.y * kY,
						   vPc.z + vSize.z * kZ);
		}

		static uint8_t childIdx(const vFloat3 &vP, const vFloat3 &vPc)
		{
			uint8_t iC = 0;
			if (vP.x >= vPc.x)
				iC |= 4;
			if (vP.y >= vPc.y)
				iC |= 2;
			if (vP.z >= vPc.z)
				iC |= 1;

			return iC;
		}

		static bool bInCell(const vFloat3 &vP, const vFloat3 &vC, const vFloat3 &vSize)
		{
			IF_F(fabs(vP.x - vC.x) > vSize.x * 0.5f);
			IF_F(fabs(vP.y - vC.y) > vSize.y * 0.5f);
			IF_F(fabs(vP.z - vC.z) > vSize.z * 0.5f);

			return true;
		}

		static void updatePclCell(OCT_PCL_CELL *pCell, const GEOMETRY_POINT &p, uint64_t tNow)
		{
			NULL_(pCell);

			if (pCell->m_nP <= 0)
				pCell->m_vC = p.m_vC;
			else
				pCell->m_vC = (pCell->m_vC * (float)pCell->m_nP + p.m_vC) / (float)(pCell->m_nP + 1);

			pCell->m_nP++;
			pCell->m_tStamp = tNow;
		}

		static bool bHasChild(OCTREE_CELL<OCT_PCL_CELL> *pCell)
		{
			NULL_F(pCell);

			for (int i = 0; i < N_OCT; i++)
			{
				IF__(pCell->getChild(i), true);
			}

			return false;
		}

		static bool bKeepCell(OCTREE_CELL<OCT_PCL_CELL> *pCell, uint64_t tExpire)
		{
			NULL_F(pCell);

			for (int i = 0; i < N_OCT; i++)
			{
				OCTREE_CELL<OCT_PCL_CELL> *pChild = pCell->getChild(i);
				IF_CONT(!pChild);

				if (!bKeepCell(pChild, tExpire))
					pCell->deleteChild(i);
			}

			OCT_PCL_CELL *pT = pCell->getT();
			if (pT && bExpired(pT->m_tStamp, tExpire))
				DEL(pCell->m_pT);

			IF__(pCell->getT(), true);
			return bHasChild(pCell);
		}

		static bool addLine(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pOut,
							const vFloat3 &vA,
							const vFloat3 &vB,
							const vFloat3 &vC,
							uint64_t tStamp,
							int *pnLine,
							int nMaxLine)
		{
			NULL_F(pOut);
			NULL_F(pOut->m_pT);
			NULL_F(pnLine);
			IF_F(*pnLine >= nMaxLine);

			GEOMETRY_LINE l;
			l.m_vPa = vA;
			l.m_vPb = vB;
			l.m_vC = vC;
			l.m_tStamp = tStamp;

			pOut->add(l);
			(*pnLine)++;

			return true;
		}

		static bool addCellLines(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pOut,
								 const vFloat3 &vC,
								 const vFloat3 &vSize,
								 const vFloat3 &vCol,
								 uint64_t tStamp,
								 int *pnLine,
								 int nMaxLine)
		{
			NULL_F(pnLine);
			IF_F(*pnLine + 12 > nMaxLine);

			vFloat3 vH = vHalf(vSize);
			vFloat3 vMin(vC.x - vH.x, vC.y - vH.y, vC.z - vH.z);
			vFloat3 vMax(vC.x + vH.x, vC.y + vH.y, vC.z + vH.z);
			vFloat3 vV[8] = {
				vFloat3(vMin.x, vMin.y, vMin.z),
				vFloat3(vMin.x, vMin.y, vMax.z),
				vFloat3(vMin.x, vMax.y, vMin.z),
				vFloat3(vMin.x, vMax.y, vMax.z),
				vFloat3(vMax.x, vMin.y, vMin.z),
				vFloat3(vMax.x, vMin.y, vMax.z),
				vFloat3(vMax.x, vMax.y, vMin.z),
				vFloat3(vMax.x, vMax.y, vMax.z),
			};

			static const uint8_t vEdge[12][2] = {
				{0, 1}, {0, 2}, {0, 4}, {1, 3},
				{1, 5}, {2, 3}, {2, 6}, {3, 7},
				{4, 5}, {4, 6}, {5, 7}, {6, 7},
			};

			for (int i = 0; i < 12; i++)
			{
				IF_F(!addLine(pOut,
							  vV[vEdge[i][0]],
							  vV[vEdge[i][1]],
							  vCol,
							  tStamp,
							  pnLine,
							  nMaxLine));
			}

			return true;
		}

		static void addCellLinesRecursive(OCTREE_CELL<OCT_PCL_CELL> *pCell,
										  const vFloat3 &vC,
										  const vFloat3 &vSize,
										  GEOMETRY_RINGBUF<GEOMETRY_LINE> *pOut,
										  const vFloat3 &vCol,
										  uint64_t tStamp,
										  int *pnLine,
										  int nMaxLine)
		{
			NULL_(pCell);
			NULL_(pnLine);
			IF_(*pnLine + 12 > nMaxLine);

			OCT_PCL_CELL *pT = pCell->getT();
			if (pT && pT->m_nP > 0 && pT->m_tStamp > 0)
			{
				IF_(!addCellLines(pOut, vC, vSize, vCol, tStamp, pnLine, nMaxLine));
			}

			vFloat3 vChildSize = vHalf(vSize);
			for (int i = 0; i < N_OCT; i++)
			{
				IF_(*pnLine + 12 > nMaxLine);

				OCTREE_CELL<OCT_PCL_CELL> *pChild = pCell->getChild(i);
				IF_CONT(!pChild);

				addCellLinesRecursive(pChild,
									  childCenter(vC, vSize, i),
									  vChildSize,
									  pOut,
									  vCol,
									  tStamp,
									  pnLine,
									  nMaxLine);
			}
		}
	}

	_OctreeGrid::_OctreeGrid()
	{
		m_vPorigin.set(0);
		m_vRootCellSize.set(10);
		m_nMaxLevel = 4;

		m_pCell = nullptr;
		m_dTexpireCell = 0;
		m_dTexpirePcl = 0;

		m_vColCellOcc.set(1);
		m_nMaxLines = 100000;
	}

	_OctreeGrid::~_OctreeGrid()
	{
		if (m_pCell)
		{
			m_pCell->release();
			delete m_pCell;
		}

		m_lnCellOcc.release();
		m_grPt.release();
	}

	bool _OctreeGrid::init(const json &j)
	{
		IF_F(!this->_OctreeBase::init(j));

		jKv<float>(j, "vPorigin", m_vPorigin);
		jKv<float>(j, "vRootCellSize", m_vRootCellSize);
		jKv(j, "nMaxLevel", m_nMaxLevel);
		jKv(j, "dTexpireCell", m_dTexpireCell);
		jKv(j, "dTexpirePcl", m_dTexpirePcl);
		jKv<float>(j, "vColCellOcc", m_vColCellOcc);
		jKv(j, "nMaxLines", m_nMaxLines);

		IF_Le_F(m_nMaxLines <= 0, "Invalid nMaxLines: " + i2str(m_nMaxLines));
		IF_Le_F(m_nMaxLevel < 0, "Invalid nMaxLevel: " + i2str(m_nMaxLevel));
		IF_Le_F(m_vRootCellSize.x <= 0.0f ||
					 m_vRootCellSize.y <= 0.0f ||
					 m_vRootCellSize.z <= 0.0f,
				 "Invalid vRootCellSize");

		int nP = 100000;
		jKv(j, "nP", nP);
		IF_Le_F(nP <= 0, "Invalid nP: " + i2str(nP));
		m_grPt.release();
		IF_Le_F(!m_grPt.alloc(nP), "Alloc failed with nP: " + i2str(nP));
		m_grPt.clear();

		m_lnCellOcc.release();
		IF_Le_F(!m_lnCellOcc.alloc(m_nMaxLines), "Alloc failed with nMaxLines: " + i2str(m_nMaxLines));
		m_lnCellOcc.clear();

		if (m_pCell)
		{
			m_pCell->release();
			delete m_pCell;
			m_pCell = nullptr;
		}
		m_pCell = new OCTREE_CELL<OCT_PCL_CELL>();
		NULL_F(m_pCell);

		return true;
	}

	bool _OctreeGrid::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_OctreeBase::link(j, pM));

		vector<string> vGn;
		jKv(j, "vGeometryBase", vGn);
		m_vpGb.clear();
		for (string n : vGn)
		{
			_GeometryBase *pG = (_GeometryBase *)(pM->findModule(n));
			IF_CONT(!pG);

			m_vpGb.push_back(pG);
		}

		return true;
	}

	bool _OctreeGrid::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _OctreeGrid::check(void)
	{
		return this->_OctreeBase::check();
	}

	void _OctreeGrid::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateGrid();
		}
	}

	void _OctreeGrid::updateGrid(void)
	{
		IF_(!check());

		updatePoint();
		deleteExpiredCells();
		updateDrawAssets();
	}

	void _OctreeGrid::updatePoint(void)
	{
		IF_(!check());
		NULL_(m_pCell);

		uint64_t tNow = getApproxTbootUs();
		uint64_t tExpire = 0;
		if (m_dTexpirePcl > 0)
			tExpire = (tNow > m_dTexpirePcl) ? tNow - m_dTexpirePcl : 0;

		for (_GeometryBase *pGb : m_vpGb)
		{
			m_grPt.clear();
			int nP = pGb->get(&m_grPt, tExpire);
			IF_CONT(nP <= 0);
			nP = small<int>(nP, m_grPt.nT());

			int i = 0;
			while (i < nP)
			{
				GEOMETRY_POINT *pGp = m_grPt.get(i++);
				if (pGp == nullptr)
					break;
				if (pGp->m_tStamp == 0)
					break;

				IF_CONT(!bInCell(pGp->m_vP, m_vPorigin, m_vRootCellSize));

				OCTREE_CELL<OCT_PCL_CELL> *pCell = m_pCell;
				vFloat3 vPc = m_vPorigin;
				vFloat3 vSize = m_vRootCellSize;

				for (int iL = 0; iL <= m_nMaxLevel; iL++)
				{
					updatePclCell(pCell->addT(), *pGp, tNow);
					if (iL >= m_nMaxLevel)
						break;

					uint8_t iC = childIdx(pGp->m_vP, vPc);
					if (!pCell->getChild(iC))
						IF_(!pCell->addChild(iC));

					OCTREE_CELL<OCT_PCL_CELL> *pChild = pCell->getChild(iC);
					IF_(!pChild);

					vPc = childCenter(vPc, vSize, iC);
					vSize = vHalf(vSize);
					pCell = pChild;
				}
			}
		}
	}

	OCT_PCL_CELL *_OctreeGrid::getCell(const vFloat3 &vP, bool bAdd)
	{
		NULL_N(m_pCell);

		IF_N(!bInCell(vP, m_vPorigin, m_vRootCellSize));

		OCTREE_CELL<OCT_PCL_CELL> *pCell = m_pCell;
		vFloat3 vPc = m_vPorigin;
		vFloat3 vSize = m_vRootCellSize;

		for (int iL = 0; iL < m_nMaxLevel; iL++)
		{
			uint8_t iC = childIdx(vP, vPc);
			OCTREE_CELL<OCT_PCL_CELL> *pChild = pCell->getChild(iC);
			if (!pChild)
			{
				IF_N(!bAdd);
				IF_N(!pCell->addChild(iC));
				pChild = pCell->getChild(iC);
				NULL_N(pChild);
			}

			vPc = childCenter(vPc, vSize, iC);
			vSize = vHalf(vSize);
			pCell = pChild;
		}

		return bAdd ? pCell->addT() : pCell->getT();
	}

	void _OctreeGrid::deleteExpiredCells(void)
	{
		IF_(!m_pCell);
		IF_(m_dTexpireCell == 0);

		uint64_t tExpire = 0;
		if (m_dTexpireCell > 0)
		{
			uint64_t tNow = getApproxTbootUs();
			tExpire = (tNow > m_dTexpireCell) ? tNow - m_dTexpireCell : 0;
		}

		bKeepCell(m_pCell, tExpire);
	}

	void _OctreeGrid::updateDrawAssets(void)
	{
		IF_(!check());
		NULL_(m_pCell);

		uint64_t tNow = getApproxTbootUs();
		int nLine = 0;

		atomicFrom();

		m_lnCellOcc.clear();
		addCellLinesRecursive(m_pCell,
							  m_vPorigin,
							  m_vRootCellSize,
							  &m_lnCellOcc,
							  m_vColCellOcc,
							  tNow,
							  &nLine,
							  m_nMaxLines);

		atomicTo();
	}

	int _OctreeGrid::get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire)
	{
		// currently the grid itself does not draw points, so we just return 0
		return 0;
	}

	int _OctreeGrid::get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pOut, uint64_t tExpire)
	{
		NULL__(pOut, 0);

		// output lines into pOut for drawing the grid cells
		atomicFrom();

		int nL = 0;
		int nLin = m_lnCellOcc.nT();
		int iL = m_lnCellOcc.iT();

		while (nL < nLin)
		{
			GEOMETRY_LINE *pGl = m_lnCellOcc.get(iL);
			if (!pGl)
				break;
			if (bExpired(pGl->m_tStamp, tExpire))
				break;

			pOut->add(*pGl);
			nL++;

			iL = m_lnCellOcc.iDec(iL);
		}

		atomicTo();

		return nL;
	}

}
