#pragma once
#ifdef USE_OPEN3D
#include "../../Base/open3d.h"
#include <set>
#include <unordered_map>
#include <unordered_set>
using namespace open3d::visualization::gui;
using namespace open3d::visualization::rendering;
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

			class PCscanUI : public gui::Window
			{
				using Super = gui::Window;

			public:
				enum class Shader
				{
					STANDARD,
					UNLIT,
					NORMALS,
					DEPTH
				};

				struct DrawObject
				{
					std::string m_name;
					std::shared_ptr<geometry::Geometry3D> m_sGeometry;
					std::shared_ptr<t::geometry::Geometry> m_sTgeometry;
					rendering::Material m_material;
					bool m_bVisible = true;
				};

				struct UIState
				{
					Shader m_sceneShader = Shader::STANDARD;
					bool m_bShowSettings = true;
					bool m_bShowAxes = true;

					Eigen::Vector4f bg_color = {0.0f, 0.0f, 0.0f, 0.0f};
					int m_pointSize = 2;
					int m_lineWidth = 2;
					SceneWidget::Controls m_mouseMode = SceneWidget::Controls::FLY;
				};

				PCscanUI(const std::string &title, int width, int height);
				virtual ~PCscanUI();

				void SetBackground(const Eigen::Vector4f &bg_color,
								   std::shared_ptr<geometry::Image> bg_image = nullptr);

				void AddGeometry(const std::string &name,
								 std::shared_ptr<geometry::Geometry3D> geom,
								 rendering::Material *material = nullptr,
								 bool is_visible = true);

				void AddPointCloud(const std::string &name,
								 std::shared_ptr<geometry::PointCloud> sPC,
								 rendering::Material *material = nullptr,
								 bool is_visible = true);

				void UpdatePointCloud(const std::string &name,
								 std::shared_ptr<geometry::PointCloud> sPC
								 );

				void RemoveGeometry(const std::string &name);

				void ShowGeometry(const std::string &name, bool show);

				DrawObject GetGeometry(const std::string &name) const;

				void SetupCamera(float fov,
								 const Eigen::Vector3f &center,
								 const Eigen::Vector3f &eye,
								 const Eigen::Vector3f &up);
				void ResetCameraToDefault();

				void ShowSettings(bool show);
				void ShowAxes(bool show);
				void SetPointSize(int point_size);
				void SetLineWidth(int line_width);

				std::vector<O3DVisualizerSelections::SelectionSet> GetSelectionSets() const;

				void ExportCurrentImage(const std::string &path);

				UIState GetUIState() const;
				rendering::Open3DScene *GetScene() const;
				const std::string m_modelName = "POINTCLOUD";

			protected:
				void Layout(const gui::Theme &theme);

			private:
				struct Impl;
				std::unique_ptr<Impl> impl_;
			};

		} // namespace visualizer
	}	  // namespace visualization
} // namespace open3d

#endif
