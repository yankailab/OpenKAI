#include "_OctreeBase.h"

namespace kai
{

	_OctreeBase::_OctreeBase()
	{
		m_type = geometry_octree;
		m_pCell = nullptr;
	}

	_OctreeBase::~_OctreeBase()
	{
		if (m_pCell)
		{
			m_pCell->release();
			delete m_pCell;
		}
	}

	bool _OctreeBase::init(const json &j)
	{
		IF_F(!this->_GeometryBase::init(j));

		loadConfig();

		return true;
	}

	bool _OctreeBase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_GeometryBase::link(j, pM));

		return true;
	}

	bool _OctreeBase::loadConfig(json *pJ, string fName)
	{
		json j;
		IF_F(!this->_GeometryBase::loadConfig(&j, fName));

		const json &jG = jK(j, "_OctreeBase");
		if (jG.is_object())
		{
			// octree config
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

		// octree config

		return this->_GeometryBase::saveConfig(j, fName);
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
