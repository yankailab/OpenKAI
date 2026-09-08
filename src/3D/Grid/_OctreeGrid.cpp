#include "_OctreeGrid.h"

namespace kai
{

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

		return true;
	}

	bool _OctreeGrid::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_OctreeBase::link(j, pM));

		vector<string> vGn;
		jKv(j, "vGeometryBase", vGn);
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
	}

	void _OctreeGrid::updatePoint(void)
	{
		IF_(!check());

		uint64_t tNow = getApproxTbootUs();
		uint64_t tExpire = 0;
		if (m_dTexpirePcl > 0)
			tExpire = tNow - m_dTexpirePcl;

		for (_GeometryBase *pGb : m_vpGb)
		{
			m_grPt.clear();
			int nP = pGb->get(&m_grPt, tExpire);
			IF_CONT(nP <= 0);

			int i = 0;
			while (i < nP)
			{
				GEOMETRY_POINT *pGp = m_grPt.get(i++);
				if (pGp == nullptr)
					break;
				if (pGp->m_tStamp == 0)
					break;

				OCT_PCL_CELL *pCell = getCell(pGp->m_vP, true); // true: create new cell instance in every levels if not existed
				IF_CONT(pCell);

				pCell->m_nP++;
				pCell->m_tStamp = tNow;
				pCell->m_vC = (pCell->m_vC + pGp->m_vC) * 0.5;
			}
		}
	}

	OCT_PCL_CELL *_OctreeGrid::getCell(const vFloat3 &vP, bool bAdd)
	{
		vFloat3 vPr = {vP.x - m_vPorigin.x, vP.y - m_vPorigin.y, vP.z - m_vPorigin.z}; // use the relative position in the local coordinate
		IF_N(abs(vPr.x) > m_vRootCellSize.x * 0.5);
		IF_N(abs(vPr.y) > m_vRootCellSize.y * 0.5);
		IF_N(abs(vPr.z) > m_vRootCellSize.z * 0.5);

		/*
			TODO: recursively iterate the m_pCell octree down to the m_nMaxLevel to find the pointer to the cell which contains point vP.
			If the cell is not existed yet and bAdd == true, create the cell needed in every level down to the m_nMaxLevel, and return the newly created cell pointer
			At each level the cell size is half at each axis of its parent level, the root level size is defined in m_vRootCellSize.
			The origin position of the cell at each level is its cubic center, with size/2 extended to each axis to make the cubic volume.
		*/

	}

	void _OctreeGrid::deleteExpiredCells(void)
	{
		uint64_t tExpire = 0;
		if (m_dTexpireCell > 0)
			tExpire = getApproxTbootUs() - m_dTexpireCell;

		/*
			TODO: recursively iterate the m_pCell octree to delete the cells with m_tStamp < tExpire
		*/
	}

	void _OctreeGrid::updateDrawAssets(void)
	{
		IF_(!check());

		// TODO: generate all the lines from m_pCell to draw the grid cells, add them into m_lnCellOcc
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

		return nL;
	}

}
