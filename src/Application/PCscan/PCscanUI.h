#pragma once
#ifdef USE_OPEN3D
#include "../../Base/open3d.h"
#include "../../Utility/utilStr.h"
#include <set>
#include <unordered_map>
#include <unordered_set>
using namespace open3d::visualization::gui;
using namespace open3d::visualization::rendering;
using namespace std;
using namespace Eigen;

typedef void (*OnBtnClickedCb)(void *pPCV, void *pD);
struct O3D_UI_Cb
{
	OnBtnClickedCb m_pCb;
	void *m_pPCV;

	void init(void)
	{
		m_pCb = NULL;
		m_pPCV = NULL;
	}

	void add(OnBtnClickedCb pCb, void *pPCV)
	{
		m_pCb = pCb;
		m_pPCV = pPCV;
	}

	bool bValid(void)
	{
		return (m_pCb && m_pPCV) ? true : false;
	}

	void call(void *pD = NULL)
	{
		if (!bValid())
			return;

		m_pCb(m_pPCV, pD);
	}
};

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
				shared_ptr<t::geometry::PointCloud> m_sTgeometry;
				rendering::Material m_material;
				bool m_bVisible = true;
			};

			class PCscanUI : public gui::Window
			{
				using Super = gui::Window;

			public:
				PCscanUI(const string &title, int width, int height);
				virtual ~PCscanUI();

				void CamSetProj(
                    double fov,
                    double aspect,
                    double near,
                    double far,
                    uint8_t fov_type
					);
				void CamSetPose(
					const Eigen::Vector3f &center,
					const Eigen::Vector3f &eye,
					const Eigen::Vector3f &up);
				void CamAutoBound(const Eigen::Vector3f &CoR);

				void ShowSettings(bool show);
				void ShowAxes(bool show);
				void SetPointSize(int point_size);
				void SetLineWidth(int line_width);
				void SetBackground(const Vector4f &bg_color,
								   shared_ptr<geometry::Image> bg_image = nullptr);

				void AddGeometry(const string &name,
								 shared_ptr<geometry::Geometry3D> geom,
								 rendering::Material *material = nullptr,
								 bool is_visible = true);

				void AddPointCloud(const string &name,
								   shared_ptr<t::geometry::PointCloud> sTg,
								   rendering::Material *material = nullptr,
								   bool is_visible = true);

				void UpdatePointCloud(const string &name,
									  shared_ptr<t::geometry::PointCloud> sTg);

				void RemoveGeometry(const string &name);
				void ShowGeometry(const string &name, bool show);
				DrawObject GetGeometry(const string &name) const;

				vector<O3DVisualizerSelections::SelectionSet> GetSelectionSets() const;

				void ExportCurrentImage(const string &path);
				rendering::Open3DScene *GetScene() const;

				void SetCbBtnScan(OnBtnClickedCb pCb, void *pPCV);
				void SetCbBtnSavePC(OnBtnClickedCb pCb, void *pPCV);
				void SetCbBtnAutoCam(OnBtnClickedCb pCb, void *pPCV);
				void SetProgressBar(float v);

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
