#pragma once
#ifdef USE_OPEN3D
#include "../../Base/open3d.h"
#include <set>
#include <unordered_map>
#include <unordered_set>
using namespace open3d::visualization::gui;
using namespace open3d::visualization::rendering;
using namespace std;
using namespace Eigen;

namespace open3d
{

	namespace geometry
	{
		class Geometry3D;
		class Image;
	}

	namespace t
	{
		namespace geometry
		{
			class Geometry;
		}
	}

	namespace visualization
	{

		namespace rendering
		{
			class Open3DScene;
		}

		namespace visualizer
		{

			enum class Shader
			{
				STANDARD,
				UNLIT,
				NORMALS,
				DEPTH
			};

			struct DrawObject
			{
				string m_name;
				shared_ptr<geometry::Geometry3D> m_sGeometry;
				shared_ptr<t::geometry::Geometry> m_sTgeometry;
				rendering::Material m_material;
				bool m_bVisible = true;
			};

			class PCscanUI : public gui::Window
			{
				using Super = gui::Window;

			public:
				PCscanUI(const string &title, int width, int height);
				virtual ~PCscanUI();

				void SetBackground(const Vector4f &bg_color,
								   shared_ptr<geometry::Image> bg_image = nullptr);

				void AddGeometry(const string &name,
								 shared_ptr<geometry::Geometry3D> geom,
								 rendering::Material *material = nullptr,
								 bool is_visible = true);

				void AddPointCloud(const string &name,
								   shared_ptr<geometry::PointCloud> sPC,
								   rendering::Material *material = nullptr,
								   bool is_visible = true);

				void UpdatePointCloud(const string &name,
									  shared_ptr<geometry::PointCloud> sPC);

				void RemoveGeometry(const string &name);

				void ShowGeometry(const string &name, bool show);

				DrawObject GetGeometry(const string &name) const;

				void SetupCamera(float fov,
								 const Vector3f &center,
								 const Vector3f &eye,
								 const Vector3f &up);
				void ResetCameraToDefault();

				void ShowSettings(bool show);
				void ShowAxes(bool show);
				void SetPointSize(int point_size);
				void SetLineWidth(int line_width);

				vector<O3DVisualizerSelections::SelectionSet> GetSelectionSets() const;

				void ExportCurrentImage(const string &path);

				rendering::Open3DScene *GetScene() const;

			protected:
				void Layout(const gui::Theme &theme);

			private:
				struct Impl;
				unique_ptr<Impl> impl_;
			};

		} // namespace visualizer
	}	  // namespace visualization
} // namespace open3d

#endif
