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
			struct DrawObject
			{
				string m_name;
				shared_ptr<geometry::Geometry3D> m_sGeometry;
				shared_ptr<t::geometry::PointCloud> m_sTgeometry;
				rendering::Material m_material;
				bool m_bVisible = true;
			};

			struct UIState
			{
				bool m_bSceneCache = false;
				bool m_bShowSettings = true;
				bool m_bShowAxes = true;

				Eigen::Vector4f bg_color = {0.0f, 0.0f, 0.0f, 0.0f};
				double m_selectPointSize = 0.025;
				int m_pointSize = 2;
				int m_lineWidth = 5;
				SceneWidget::Controls m_mouseMode = SceneWidget::Controls::FLY;
			};

			class PCscanUI : public gui::Window
			{
				using Super = gui::Window;

			public:
				PCscanUI(const string &title, int width, int height);
				virtual ~PCscanUI();

				void Init(void);

				void AddGeometry(const string &name,
								 shared_ptr<geometry::Geometry3D> sTg,
								 rendering::Material *material = nullptr,
								 bool bVisible = true);
				void AddPointCloud(const string &name,
								   shared_ptr<t::geometry::PointCloud> sTg,
								   rendering::Material *material = nullptr,
								   bool bVisible = true);
				void UpdatePointCloud(const string &name,
									  shared_ptr<t::geometry::PointCloud> sTg);
				void RemoveGeometry(const string &name);

				void CamSetProj(
					double fov,
					double aspect,
					double near,
					double far,
					uint8_t fov_type);
				void CamSetPose(
					const Eigen::Vector3f &center,
					const Eigen::Vector3f &eye,
					const Eigen::Vector3f &up);
				void CamAutoBound(const Eigen::Vector3f &CoR);

				void UpdateUIsettings(void);
				void SetBackground(const Vector4f &bg_color,
								   shared_ptr<geometry::Image> bg_image = nullptr);
				void ShowSettings(bool show);
				void ShowAxes(bool show);
            	void SetPointSize(int px);
    	        void SetLineWidth(int px);

	            void SetMouseCameraMode(void);
	            void SetMousePickingMode(void);
				void SetCbBtnScan(OnBtnClickedCb pCb, void *pPCV);
				void SetCbBtnSavePC(OnBtnClickedCb pCb, void *pPCV);
				void SetCbBtnCamReset(OnBtnClickedCb pCb, void *pPCV);
				void SetProgressBar(float v);
	            void SetLabelArea(const string &s);

				DrawObject GetGeometry(const string &name) const;
				vector<O3DVisualizerSelections::SelectionSet> GetSelectionSets() const;
				UIState *getUIState(void);
				rendering::Open3DScene *GetScene() const;
				void ExportCurrentImage(const string &path);

			protected:
				void Layout(const gui::Theme &theme);
	            void InitCtrlPanel(void);

	            void UpdateTgeometry(const string &name, shared_ptr<t::geometry::PointCloud> sTg);

	            float ConvertToScaledPixels(int px);
	            void UpdateSelectableGeometry(void);

	            void NewVertexSet(void);
            	void SelectVertexSet(int index);
	            void UpdateVertexSetList(void);
            	void RemoveVertexSet(int index);
	            void RemoveAllVertexSet(void);

				void UpdateArea(void);
				void RemoveDistLabel(void);
				double Area(vector<Vector3d> &vP);

				void OnExportRGB(void);
				void OnExportPLY(void);

			private:
				vector<DrawObject> m_vObject;
				shared_ptr<O3DVisualizerSelections> m_sVertex;

				UIState m_uiState;
				Window *m_pWindow = nullptr;
				SceneWidget *m_pScene = nullptr;
				bool m_bScanning;
				string m_modelName;
				string m_areaName;
				Vector3d m_areaLineCol;
				vector<shared_ptr<Label3D>> m_vspDistLabel;

				//UI components
				Vert *m_panelCtrl;
				Button *m_btnScanStart;
				ProgressBar *m_progScan;
				Label *m_labelProg;
				Button *m_btnNewVertexSet;
				Button *m_btnDeleteVertexSet;
				ListView *m_listVertexSet;
				Label *m_labelArea;
				Slider *m_sliderPointSize;

				//UI handler
				O3D_UI_Cb m_cbBtnScan;
				O3D_UI_Cb m_cbBtnSavePC;
				O3D_UI_Cb m_cbBtnCamReset;
			};

		} // namespace visualizer
	}	  // namespace visualization
} // namespace open3d

#endif
