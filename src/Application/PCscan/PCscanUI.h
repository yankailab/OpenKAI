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
	OnBtnClickedCb m_pCb = NULL;
	void *m_pPCV = NULL;

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
				SceneWidget::Controls m_mouseMode = SceneWidget::Controls::FLY;
				bool m_bSceneCache = false;
				bool m_bShowPanel = true;
				bool m_bShowAxes = true;

				int m_panelWidth = 15;
				int m_pointSize = 2;
				double m_selectPointSize = 0.025;
				double m_voxelSize = 0.01;
				int m_lineWidth = 5;
				Vector4f m_vBgCol = {0.0f, 0.0f, 0.0f, 0.0f};
				Vector3f m_vSunDir = {0.0f, 0.0f, 0.0f};
                Vector3d m_vAreaLineCol = Vector3d(1.0, 0.0, 1.0);

				Vector3f m_vCamPos;
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
					const Vector3f &center,
					const Vector3f &eye,
					const Vector3f &up);

				void CamAutoBound(const geometry::AxisAlignedBoundingBox& aabb,
                                  const Vector3f &CoR);

				UIState *getUIState(void);
				void UpdateUIstate(void);
            	void SetPointSize(int px);
    	        void SetLineWidth(int px);

				DrawObject GetGeometry(const string &name) const;
				vector<O3DVisualizerSelections::SelectionSet> GetSelectionSets() const;
				rendering::Open3DScene *GetScene() const;
				void ExportCurrentImage(const string &path);

	            void SetMouseCameraMode(void);
	            void SetMousePickingMode(void);
				void SetProgressBar(float v);
	            void SetLabelArea(const string &s);
				void SetCbBtnScan(OnBtnClickedCb pCb, void *pPCV);
				void SetCbBtnOpenPC(OnBtnClickedCb pCb, void *pPCV);
				void SetCbBtnCamSet(OnBtnClickedCb pCb, void *pPCV);
				void SetCbVoxelDown(OnBtnClickedCb pCb, void *pPCV);
				void SetCbBtnHiddenRemove(OnBtnClickedCb pCb, void *pPCV);
				void SetCbBtnResetPC(OnBtnClickedCb pCb, void *pPCV);

				void ShowMsg(const char* pTitle, const char* pMsg, bool bOK = false);
				void CloseMsg(void);

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

				void OnSaveRGB(void);
				void OnSavePLY(void);
				void OnOpenPLY(void);

			private:
				vector<DrawObject> m_vObject;
				shared_ptr<O3DVisualizerSelections> m_sVertex;

				Window *m_pWindow = nullptr;
				SceneWidget *m_pScene = nullptr;
				UIState m_uiState;
				string m_modelName;
				string m_areaName;
				bool m_bScanning;
				vector<shared_ptr<Label3D>> m_vspDistLabel;

				//UI components
				Vert *m_panelCtrl;
				Button *m_btnScanStart;

				Slider* m_sliderVsize;
				Button* m_btnHiddenRemove;
				Button* m_btnFilterReset;

				ProgressBar *m_progScan;
				Label *m_labelProg;
				Button *m_btnNewVertexSet;
				Button *m_btnDeleteVertexSet;
				ListView *m_listVertexSet;
				Label *m_labelArea;

				//UI handler
				O3D_UI_Cb m_cbBtnScan;
				O3D_UI_Cb m_cbBtnOpenPC;
				O3D_UI_Cb m_cbBtnCamSet;
				O3D_UI_Cb m_cbVoxelDown;
				O3D_UI_Cb m_cbBtnHiddenRemove;
				O3D_UI_Cb m_cbBtnResetPC;
			};

		} // namespace visualizer
	}	  // namespace visualization
} // namespace open3d

#endif
