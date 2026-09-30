#ifndef OpenKAI_src_UI_Viewer_Web_WebGeometryCache_H_
#define OpenKAI_src_UI_Viewer_Web_WebGeometryCache_H_

#include "../SelectableOctGridSources.h"
#include "WebSelectableOctGridProtocol.h"
#include <algorithm>
#include <cmath>

namespace kai
{
	// A changed source timestamp copies and encodes the frame. The earliest visible record
	// timestamp schedules the next expiry check without scanning static geometry.
	class WebGeometryCache
	{
	public:
		bool refresh(const VIEWER_GEOMETRY_SOURCE &source, webselectableoctgrid::Type type,
			uint32_t id, uint64_t expiry)
		{
			const bool points = type == webselectableoctgrid::Type::Points;
			const uint64_t stamp = points
				? (source.m_pPCLframein ? source.m_pPCLframein->getTstamp() : 0)
				: (source.m_pLineFramein ? source.m_pLineFramein->getTstamp() : 0);
			const bool changed = m_bytes.empty() || stamp != m_tStamp;
			if (!changed && expiry <= m_tFirstVisible)
			{
				return false;
			}

			if (changed)
			{
				m_tStamp = stamp;
				if (points && source.m_pPCLframein)
				{
					m_tStamp = source.m_pPCLframein->get(m_points);
				}
				else if (!points && source.m_pLineFramein)
				{
					m_tStamp = source.m_pLineFramein->get(m_lines);
				}
			}

			m_positions.clear();
			m_colors.clear();
			m_tFirstVisible = UINT64_MAX;
			float bounds[6] = {FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};
			if (points)
			{
				for (const auto &point : m_points)
				{
					if (m_positions.size() / 3 >= size_t(source.m_nP))
					{
						break;
					}
					if (!point.m_tStamp || point.m_tStamp < expiry || !point.m_vP.allFinite())
					{
						continue;
					}
					vertex(point.m_vP, point.m_vC, source.m_matCol, bounds);
					m_tFirstVisible = std::min(m_tFirstVisible, point.m_tStamp);
				}
			}
			if (!points)
			{
				for (const auto &line : m_lines)
				{
					if (m_positions.size() / 6 >= size_t(source.m_nL))
					{
						break;
					}
					if (!line.m_tStamp || line.m_tStamp < expiry || !line.m_vPa.allFinite() || !line.m_vPb.allFinite())
					{
						continue;
					}
					vertex(line.m_vPa, line.m_vC, source.m_matCol, bounds);
					vertex(line.m_vPb, line.m_vC, source.m_matCol, bounds);
					m_tFirstVisible = std::min(m_tFirstVisible, line.m_tStamp);
				}
			}
			if (m_positions.empty())
			{
				std::fill(bounds, bounds + 6, 0.f);
			}

			webselectableoctgrid::begin(m_bytes, type, 0, 0);
			if (points)
			{
				webselectableoctgrid::points(m_bytes, id, source.m_matPointSize, bounds, m_positions, m_colors);
			}
			else
			{
				webselectableoctgrid::lines(m_bytes, id, bounds, m_positions, m_colors);
			}
			return true;
		}

		void appendTo(std::vector<uint8_t> &frame) const
		{
			frame.insert(frame.end(), m_bytes.begin() + webselectableoctgrid::HeaderBytes, m_bytes.end());
		}

	private:
		static uint8_t colorByte(float color)
		{
			return uint8_t(std::clamp(std::isfinite(color) ? color : 1.f, 0.f, 1.f) * 255.f + .5f);
		}

		void vertex(const Vector3f &position, Vector3f color, const Vector4f &fallback, float bounds[6])
		{
			m_positions.insert(m_positions.end(), {position.x(), position.y(), position.z()});
			if (color.x() <= 0 && color.y() <= 0 && color.z() <= 0)
			{
				color = fallback.head<3>();
			}
			m_colors.insert(m_colors.end(), {colorByte(color.x()), colorByte(color.y()), colorByte(color.z())});
			for (size_t axis = 0; axis < 3; ++axis)
			{
				bounds[axis] = std::min(bounds[axis], position[axis]);
				bounds[axis + 3] = std::max(bounds[axis + 3], position[axis]);
			}
		}

		std::vector<GEOMETRY_POINT> m_points;
		std::vector<GEOMETRY_LINE> m_lines;
		uint64_t m_tStamp = 0;
		uint64_t m_tFirstVisible = UINT64_MAX;
		std::vector<uint8_t> m_bytes;
		std::vector<float> m_positions;
		std::vector<uint8_t> m_colors;
	};
}
#endif
