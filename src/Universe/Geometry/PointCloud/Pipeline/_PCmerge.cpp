/*
 * _PCmerge.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PCmerge.h"
#include <cmath>
#include <unordered_map>

namespace kai
{
	namespace
	{
		struct MergeVoxel
		{
			int64_t m_x = 0;
			int64_t m_y = 0;
			int64_t m_z = 0;

			bool set(const Vector3f &position, float size)
			{
				const Eigen::Vector3d index = (position.cast<double>() / size).array().floor();
				// Strict bounds avoid conversion overflow at rounded INT64_MAX.
				const double limit = static_cast<double>(INT64_MAX);
				if (!index.allFinite() || (index.array() <= -limit).any() || (index.array() >= limit).any())
				{
					return false;
				}
				m_x = static_cast<int64_t>(index.x());
				m_y = static_cast<int64_t>(index.y());
				m_z = static_cast<int64_t>(index.z());
				return true;
			}

			bool operator==(const MergeVoxel &other) const
			{
				return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z;
			}
		};

		struct MergeVoxelHash
		{
			size_t operator()(const MergeVoxel &voxel) const
			{
				size_t hash = std::hash<int64_t>{}(voxel.m_x);
				hash ^= std::hash<int64_t>{}(voxel.m_y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
				hash ^= std::hash<int64_t>{}(voxel.m_z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
				return hash;
			}
		};
	}

	_PCmerge::_PCmerge()
	{
	}

	_PCmerge::~_PCmerge()
	{
	}

	bool _PCmerge::loadConfig(void)
	{
		IF_F(!this->_PointCloud::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "rVoxel", m_rVoxel);
		IF_Le_F(!std::isfinite(m_rVoxel) || m_rVoxel < 0.0f, "Invalid rVoxel");
		return true;
	}

	bool _PCmerge::saveConfig(bool bExport)
	{
		IF_F(!_PointCloud::saveConfig(false));

		json &j = *m_pJ;
		j["rVoxel"] = m_rVoxel;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _PCmerge::link(InstanceMgr *pM)
	{
		IF_F(!this->_PointCloud::link(pM));
		const json &j = *m_pJ;

		vector<string> names;
		jKv(j, "vPCLframes", names);
		IF_Le_F(names.empty(), "vPCLframes must contain at least one input");
		m_vpPCL.clear();
		for (const string &name : names)
		{
			PCLframe *pFrame = dynamic_cast<PCLframe *>(static_cast<DataStreamBase *>(pM->findDataStream(name)));
			IF_Le_F(!pFrame, "PCLframe not found: " + name);
			IF_Le_F(pFrame == m_pPCL, "Merge input must differ from PCLframe output");
			m_vpPCL.push_back(pFrame);
		}
		m_vInputRevision.assign(m_vpPCL.size(), 0);

		return true;
	}

	bool _PCmerge::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _PCmerge::check(void)
	{
		return this->_PointCloud::check();
	}

	void _PCmerge::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateMerge();
		}
	}

	void _PCmerge::updateMerge(void)
	{
		IF_(!check());

		vector<PCLframe::SnapshotPtr> frames;
		frames.reserve(m_vpPCL.size());
		bool changed = false;
		size_t count = 0;
		uint64_t stamp = 0;
		for (size_t i = 0; i < m_vpPCL.size(); ++i)
		{
			PCLframe::SnapshotPtr frame = m_vpPCL[i]->get();
			changed = changed || frame->m_revision != m_vInputRevision[i];
			count += frame->m_vPoints.size();
			stamp = std::max(stamp, frame->m_tStamp);
			frames.push_back(std::move(frame));
		}
		if (!changed)
		{
			return;
		}

		vector<GEOMETRY_POINT> points;
		points.reserve(count);
		std::unordered_map<MergeVoxel, size_t, MergeVoxelHash> voxels;
		vector<size_t> voxelCounts;
		for (const PCLframe::SnapshotPtr &frame : frames)
		{
			for (const GEOMETRY_POINT &point : frame->m_vPoints)
			{
				if (point.m_tStamp == 0)
				{
					continue;
				}

				GEOMETRY_POINT transformed = point;
				transformed.m_vP = m_mPosef * point.m_vP;
				if (m_rVoxel == 0.0f)
				{
					points.push_back(transformed);
					continue;
				}

				MergeVoxel voxel;
				if (!voxel.set(transformed.m_vP, m_rVoxel))
				{
					continue;
				}
				const auto inserted = voxels.emplace(voxel, points.size());
				if (inserted.second)
				{
					points.push_back(transformed);
					voxelCounts.push_back(1);
					continue;
				}

				const size_t index = inserted.first->second;
				GEOMETRY_POINT &merged = points[index];
				const float weight = 1.0f / static_cast<float>(++voxelCounts[index]);
				merged.m_vP += (transformed.m_vP - merged.m_vP) * weight;
				merged.m_vC += (transformed.m_vC - merged.m_vC) * weight;
				merged.m_tStamp = std::max(merged.m_tStamp, transformed.m_tStamp);
			}
		}

		m_pPCL->set(std::move(points), stamp);
		for (size_t i = 0; i < frames.size(); ++i)
		{
			m_vInputRevision[i] = frames[i]->m_revision;
		}
	}

}
