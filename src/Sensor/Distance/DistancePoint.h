#ifndef OpenKAI_src_Sensor_Distance_DistancePoint_H_
#define OpenKAI_src_Sensor_Distance_DistancePoint_H_

#include "../../DataObject/PCLframe.h"
#include <cmath>

namespace kai
{
	inline GEOMETRY_POINT distancePoint(float d, float a, int iDiv, int nDiv,
		float fovH, const Vector2f &vRange, uint64_t tStamp)
	{
		GEOMETRY_POINT point;
		point.m_vC.x() = a;
		if (iDiv < 0 || iDiv >= nDiv || !std::isfinite(d) ||
			d < 0 || d < vRange.x() || d >= vRange.y())
		{
			return point;
		}

		// Meters in the horizontal XY plane, +X forward. Use division centers
		// so a single beam points forward regardless of the configured FOV.
		float angle = ((iDiv + 0.5f) / nDiv - 0.5f) * fovH * DEG_2_RAD;
		point.m_vP = Vector3f(d * std::cos(angle), d * std::sin(angle), 0);
		point.m_tStamp = tStamp;
		return point;
	}
}

#endif
