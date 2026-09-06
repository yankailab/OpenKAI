#include "_OctreeBase.h"

namespace kai
{

	_OctreeBase::_OctreeBase()
	{
		m_type = geometry_octree;

		m_vPorigin.clear();
		m_dTexpire = 0;
	}

	_OctreeBase::~_OctreeBase()
	{
	}

	bool _OctreeBase::init(const json &j)
	{
		IF_F(!this->_GeometryBase::init(j));

		jKv<double>(j, "vPorigin", m_vPorigin);
		jKv(j, "dTexpire", m_dTexpire);

		loadConfig();

		return true;
	}

	bool _OctreeBase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_GeometryBase::link(j, pM));

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

	bool _OctreeBase::loadConfig(json *pJ, string fName)
	{
		json j;
		IF_F(!this->_GeometryBase::loadConfig(&j, fName));

		const json &jG = jK(j, "_OctreeBase");
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

	bool _OctreeBase::saveConfig(json &j, string fName)
	{
		json jG = json::object();

		// grid config
		jG["vPorigin"] = {m_vPorigin.x, m_vPorigin.y, m_vPorigin.z};

		jG["dTexpire"] = m_dTexpire;

		j["_OctreeBase"] = jG;

		return this->_GeometryBase::saveConfig(j, fName);
	}

	bool _OctreeBase::initGeometry(void)
	{

		return true;
	}

	bool _OctreeBase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _OctreeBase::check(void)
	{
		return this->_GeometryBase::check();
	}

	void _OctreeBase::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			atomicFrom();

			updateOctree();

			atomicTo();
		}
	}

	void _OctreeBase::updateOctree(void)
	{
		IF_(!check());

	}

}
