#ifndef OpenKAI_src_Universe_Geometry_PointCloud_Registration_PCLframeToOpen3D_H_
#define OpenKAI_src_Universe_Geometry_PointCloud_Registration_PCLframeToOpen3D_H_

#include "../../../../DataObject/PCLframe.h"
#include <open3d/geometry/PointCloud.h>

namespace kai
{
	inline open3d::geometry::PointCloud pclFrameToOpen3D(PCLframe &frame)
	{
		vector<GEOMETRY_POINT> points;
		frame.get(points);
		open3d::geometry::PointCloud cloud;
		cloud.points_.reserve(points.size());
		cloud.colors_.reserve(points.size());
		for (const GEOMETRY_POINT &point : points)
		{
			if (point.m_tStamp == 0)
			{
				continue;
			}
			cloud.points_.push_back(point.m_vP.cast<double>());
			cloud.colors_.push_back(point.m_vC.cast<double>());
		}
		return cloud;
	}
}

#endif
