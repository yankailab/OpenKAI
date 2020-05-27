/*
 * _PointCloudBase.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PointCloudBase.h"

#ifdef USE_OPEN3D

namespace kai
{

_PointCloudBase::_PointCloudBase()
{
	m_bOpen = false;
	m_type = pointCloud_unknown;
}

_PointCloudBase::~_PointCloudBase()
{
}

bool _PointCloudBase::init(void *pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	string fName = "";
	pK->v("fName", &fName);

	utility::SetVerbosityLevel(utility::VerbosityLevel::Debug);

//	m_pPC = io::CreatePointCloudFromFile(fName.c_str());

	IF_F(!io::ReadPointCloud(fName.c_str(), m_pc));

	std::shared_ptr<geometry::PointCloud> pointcloud_ptr(new geometry::PointCloud);
	*pointcloud_ptr = m_pc;
	pointcloud_ptr->NormalizeNormals();
	auto bounding_box = pointcloud_ptr->GetAxisAlignedBoundingBox();

	std::shared_ptr<geometry::PointCloud> pointcloud_transformed_ptr(new geometry::PointCloud);
	*pointcloud_transformed_ptr = *pointcloud_ptr;
	Eigen::Matrix4d trans_to_origin = Eigen::Matrix4d::Identity();
	trans_to_origin.block<3, 1>(0, 3) = bounding_box.GetCenter() * -1.0;
	Eigen::Matrix4d transformation = Eigen::Matrix4d::Identity();
	transformation.block<3, 3>(0, 0) =
			static_cast<Eigen::Matrix3d>(Eigen::AngleAxisd(M_PI / 4.0,
					Eigen::Vector3d::UnitX()));
	pointcloud_transformed_ptr->Transform(
			trans_to_origin.inverse() * transformation * trans_to_origin);

	m_visualizer.CreateVisualizerWindow("Open3D", 1600, 900);
	m_visualizer.AddGeometry(pointcloud_ptr);
	m_visualizer.AddGeometry(pointcloud_transformed_ptr);
	m_visualizer.Run();
	m_visualizer.DestroyVisualizerWindow();

	// 4. test operations
	*pointcloud_transformed_ptr += *pointcloud_ptr;
	visualization::DrawGeometries(
	{ pointcloud_transformed_ptr }, "Combined Pointcloud");

	// 5. test downsample
	auto downsampled = pointcloud_ptr->VoxelDownSample(0.05);
	visualization::DrawGeometries(
	{ downsampled }, "Down Sampled Pointcloud");

	// 6. test normal estimation
	visualization::DrawGeometriesWithKeyCallbacks(
	{ pointcloud_ptr },
			{
			{ GLFW_KEY_SPACE, [&](visualization::Visualizer *vis)
			{
				// EstimateNormals(*pointcloud_ptr,
				//        open3d::KDTreeSearchParamKNN(20));
				pointcloud_ptr->EstimateNormals(
						open3d::geometry::KDTreeSearchParamRadius(0.05));
				utility::LogInfo("Done.");
				return true;
			} } }, "Press Space to Estimate Normal", 1600, 900);

	return true;
}

int _PointCloudBase::size(void)
{
	return -1;
}

POINTCLOUD_TYPE _PointCloudBase::getType(void)
{
	return m_type;
}

bool _PointCloudBase::open(void)
{
	return true;
}

bool _PointCloudBase::isOpened(void)
{
	return m_bOpen;
}

void _PointCloudBase::close(void)
{
	m_bOpen = false;
}

void _PointCloudBase::draw(void)
{
	this->_ThreadBase::draw();

/*
	bool pointcloud_has_normal = m_pPC->HasNormals();
	utility::LogInfo("Pointcloud has %d points.", (int) m_pPC->points_.size());

	Eigen::Vector3d min_bound = m_pPC->GetMinBound();
	Eigen::Vector3d max_bound = m_pPC->GetMaxBound();
	utility::LogInfo(
			"Bounding box is: ({:.4f}, {:.4f}, {:.4f}) - ({:.4f}, {:.4f}, "
					"{:.4f})", min_bound(0), min_bound(1), min_bound(2),
			max_bound(0), max_bound(1), max_bound(2));

	for (size_t i = 0; i < m_pPC->points_.size(); i++)
	{
		if (pointcloud_has_normal)
		{
			const Eigen::Vector3d &point = m_pPC->points_[i];
			const Eigen::Vector3d &normal = m_pPC->normals_[i];
			utility::LogInfo("{:.6f} {:.6f} {:.6f} {:.6f} {:.6f} {:.6f}",
					point(0), point(1), point(2), normal(0), normal(1),
					normal(2));
		}
		else
		{
			const Eigen::Vector3d &point = m_pPC->points_[i];
			utility::LogInfo("{:.6f} {:.6f} {:.6f}", point(0), point(1),
					point(2));
		}
	}
	utility::LogInfo("End of the list.");
*/
}

}
#endif
