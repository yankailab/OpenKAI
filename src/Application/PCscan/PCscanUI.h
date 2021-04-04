#pragma once
#ifdef USE_OPEN3D
#include "../../Base/open3d.h"
#include <set>
#include <unordered_map>
#include <unordered_set>
using namespace open3d::visualization::gui;
using namespace open3d::visualization::rendering;

namespace open3d
{

	namespace geometry
	{
		class Geometry3D;
		class Image;
	} // namespace geometry

	namespace t
	{
		namespace geometry
		{
			class Geometry;
		} // namespace geometry
	}	  // namespace t

	namespace visualization
	{

		namespace rendering
		{
			class Open3DScene;
		} // namespace rendering

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
					std::string name;
					std::shared_ptr<geometry::Geometry3D> geometry;
					std::shared_ptr<t::geometry::Geometry> tgeometry;
					rendering::Material material;
					std::string group;
					double time = 0.0;
					bool is_visible = true;

					// internal
					bool is_color_default = true;
				};

				struct UIState
				{
					Shader scene_shader = Shader::STANDARD;
					bool show_settings = false;
					bool show_skybox = false;
					bool show_axes = false;
					bool is_animating = false;
					std::set<std::string> enabled_groups;

					Eigen::Vector4f bg_color = {0.0f, 0.0f, 0.0f, 0.0f};
					int point_size = 3;
					int line_width = 2;

					bool use_ibl = false;
					bool use_sun = true;
					std::string ibl_path = ""; // "" is default path
					int ibl_intensity = 0;
					int sun_intensity = 100000;
					Eigen::Vector3f sun_dir = {0.577f, -0.577f, -0.577f};
					Eigen::Vector3f sun_color = {1.0f, 1.0f, 1.0f};

					double current_time = 0.0;	// seconds
					double time_step = 1.0;		// seconds
					double frame_delay = 0.100; // seconds
				};

				PCscanUI(const std::string &title, int width, int height);
				virtual ~PCscanUI();

				void SetBackground(const Eigen::Vector4f &bg_color,
								   std::shared_ptr<geometry::Image> bg_image = nullptr);

				void SetShader(Shader shader);

				void AddGeometry(const std::string &name,
								 std::shared_ptr<geometry::Geometry3D> geom,
								 rendering::Material *material = nullptr,
								 const std::string &group = "",
								 double time = 0.0,
								 bool is_visible = true);

				void AddGeometry(const std::string &name,
								 std::shared_ptr<t::geometry::Geometry> tgeom,
								 rendering::Material *material = nullptr,
								 const std::string &group = "",
								 double time = 0.0,
								 bool is_visible = true);

				void AddPointCloud(const std::string &name,
								 std::shared_ptr<geometry::PointCloud> sPC,
								 rendering::Material *material = nullptr,
								 const std::string &group = "",
								 double time = 0.0,
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
				void ShowSkybox(bool show);
				void ShowAxes(bool show);
				void SetPointSize(int point_size);
				void SetLineWidth(int line_width);
				void EnableGroup(const std::string &group, bool enable);

				std::vector<O3DVisualizerSelections::SelectionSet> GetSelectionSets() const;

				void ExportCurrentImage(const std::string &path);

				UIState GetUIState() const;
				rendering::Open3DScene *GetScene() const;

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
