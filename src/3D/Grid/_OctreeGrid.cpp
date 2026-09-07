#include "_OctreeGrid.h"

namespace kai
{

	_OctreeGrid::_OctreeGrid()
	{
		m_vPorigin.clear();
		m_vRootCellSize.set(10);
		m_dTexpire = 0;
	}

	_OctreeGrid::~_OctreeGrid()
	{
	}

	bool _OctreeGrid::init(const json &j)
	{
		IF_F(!this->_OctreeBase::init(j));

		jKv<double>(j, "vPorigin", m_vPorigin);
		jKv<float>(j, "vRootCellSize", m_vRootCellSize);
		jKv(j, "dTexpire", m_dTexpire);

		loadConfig();

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

	bool _OctreeGrid::loadConfig(json *pJ, string fName)
	{
		json j;
		IF_F(!this->_OctreeBase::loadConfig(&j, fName));

		const json &jG = jK(j, "_OctreeGrid");
		if (jG.is_object())
		{
			// grid config
			jKv<double>(jG, "vPorigin", m_vPorigin);
			jKv(jG, "dTexpire", m_dTexpire);
		}

		if (pJ)
		{
			*pJ = j;
		}
		return true;
	}

	bool _OctreeGrid::saveConfig(json &j, string fName)
	{
		json jG = json::object();

		// grid config
		jG["vPorigin"] = {m_vPorigin.x, m_vPorigin.y, m_vPorigin.z};
		jG["vRootCellSize"] = {m_vRootCellSize.x, m_vRootCellSize.y, m_vRootCellSize.z};
		jG["dTexpire"] = m_dTexpire;

		j["_OctreeGrid"] = jG;

		return this->_OctreeBase::saveConfig(j, fName);
	}

	void _OctreeGrid::addPointCloud(void *p, const uint64_t tExpire)
	{
	}

	int _OctreeGrid::get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pGrPout, uint64_t tExpire)
	{
		return 0;
	}

	int _OctreeGrid::get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pGrLOut, uint64_t tExpire)
	{
		return 0;
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

			atomicFrom();

			updateOctreeGrid();

			atomicTo();
		}
	}

	void _OctreeGrid::updateOctreeGrid(void)
	{
		IF_(!check());
	}

}
