#include "_OctreeGridBase.h"

namespace kai
{
	bool octgridValidCellID(const UUID128 &id, int maxLevel)
	{
		const unsigned depth = (id.m_uint64[1] >> 56) & 63;
		IF_F(maxLevel < 0 || maxLevel > OCTGRID_MAX_LEVEL || depth > unsigned(maxLevel) || id.m_uint64[1] >> 62);
		const unsigned unused = 3 * (OCTGRID_MAX_LEVEL - depth);
		if (unused >= 64)
		{
			IF_F(id.m_uint64[0] || (id.m_uint64[1] & ((uint64_t(1) << (unused - 64)) - 1)));
		}
		else if (unused)
		{
			IF_F(id.m_uint64[0] & ((uint64_t(1) << unused) - 1));
		}
		return true;
	}

	unsigned octgridCellChild(const UUID128 &id, unsigned level)
	{
		IF__(level >= OCTGRID_MAX_LEVEL, 0);
		const unsigned bit = 117 - level * 3;
		const unsigned word = bit / 64, shift = bit % 64;
		return ((id.m_uint64[word] >> shift) |
				(word == 0 && shift > 61 ? id.m_uint64[1] << (64 - shift) : 0)) & 7;
	}

	bool octgridChildID(const UUID128 &parent, uint8_t child, UUID128 &id)
	{
		IF_F(child >= N_OCT || !octgridValidCellID(parent, OCTGRID_MAX_LEVEL - 1));
		const unsigned depth = (parent.m_uint64[1] >> 56) & 63;
		id = parent;
		id.m_uint64[1] = (id.m_uint64[1] & (UINT64_MAX >> 8)) | (uint64_t(depth + 1) << 56);
		id |= UUID128(child) << (117 - depth * 3);
		return true;
	}

	bool octgridCellBox(const std::array<float, 3> &origin, const std::array<float, 3> &rootSize,
					   int maxLevel, const UUID128 &id, std::array<float, 3> &center, std::array<float, 3> &size)
	{
		IF_F(!octgridValidCellID(id, maxLevel));
		const unsigned depth = (id.m_uint64[1] >> 56) & 63;
		std::array<double, 3> c = {origin[0], origin[1], origin[2]};
		std::array<double, 3> s = {rootSize[0], rootSize[1], rootSize[2]};
		for (unsigned level = 0; level < depth; ++level)
		{
			const unsigned child = octgridCellChild(id, level);
			for (unsigned axis = 0; axis < 3; ++axis)
			{
				c[axis] += s[axis] * ((child & (4 >> axis)) ? 0.25 : -0.25);
				s[axis] *= 0.5;
			}
		}
		for (unsigned axis = 0; axis < 3; ++axis)
		{
			center[axis] = float(c[axis]);
			size[axis] = float(s[axis]);
		}
		return true;
	}

	_OctreeGridBase::_OctreeGridBase()
	{
	}

	_OctreeGridBase::~_OctreeGridBase()
	{
	}

	bool _OctreeGridBase::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;
		jKv<float>(j, "vPorigin", m_vPorigin);
		jKv<float>(j, "vRootCellSize", m_vRootCellSize);
		jKv(j, "nMaxLevel", m_nMaxLevel);

		IF_Le_F(m_nMaxLevel < 0 || m_nMaxLevel > OCTGRID_MAX_LEVEL, "Invalid nMaxLevel: " + i2str(m_nMaxLevel));
		IF_Le_F(!m_vPorigin.allFinite(), "Invalid vPorigin");
		IF_Le_F(!m_vRootCellSize.allFinite() || m_vRootCellSize.x() <= 0.0f ||
				m_vRootCellSize.y() <= 0.0f || m_vRootCellSize.z() <= 0.0f, "Invalid vRootCellSize");

		return true;
	}

	bool _OctreeGridBase::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));
		json &j = *m_pJ;
		j["vPorigin"] = {m_vPorigin.x(), m_vPorigin.y(), m_vPorigin.z()};
		j["vRootCellSize"] = {m_vRootCellSize.x(), m_vRootCellSize.y(), m_vRootCellSize.z()};
		j["nMaxLevel"] = m_nMaxLevel;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _OctreeGridBase::link(InstanceMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(pM));

		return true;
	}

	bool _OctreeGridBase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _OctreeGridBase::check(void)
	{
		return this->_ModuleBase::check();
	}

	void _OctreeGridBase::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateOctree();
		}
	}

	void _OctreeGridBase::updateOctree(void)
	{
		IF_(!check());
	}

}
