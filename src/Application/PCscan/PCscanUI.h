#pragma once
#ifdef USE_OPEN3D
#include "../../Base/open3d.h"
#include "../../Utility/util.h"
#include "../../Utility/utilStr.h"
#include <set>
#include <unordered_map>
#include <unordered_set>
using namespace open3d::visualization::gui;
using namespace open3d::visualization::rendering;
using namespace std;
using namespace Eigen;
using namespace kai;

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

			enum UImode
			{
				uiMode_cam = 0,
				uiMode_pointPick = 1,
			};

			struct UIState
			{
				//general
				bool m_bSceneCache = false;
				bool m_bShowPanel = true;
				bool m_bShowAxes = true;

				//render
				int m_wPanel = 15;
				int m_sPoint = 2;
				int m_wLine = 10;
				Vector4f m_vBgCol = {0.0f, 0.0f, 0.0f, 0.0f};
				Vector3f m_vSunDir = {0.0f, 0.0f, 0.0f};
                Vector3d m_vAreaLineCol = {1.0, 0.0, 1.0};

				//filter
				double m_sVoxel = 0.01;
				int m_oRemovN = 10;
				double m_oRemovD = 0.1;

				//interactive
				Vector3f m_vCamPos;
				float m_dMove = 0.5;
				SceneWidget::Controls m_mouseMode = SceneWidget::Controls::ROTATE_CAMERA;
			};

			class PCscanUI : public gui::Window
			{
				using Super = gui::Window;

			public:
				PCscanUI(const string &title, int width, int height);
				virtual ~PCscanUI();

				void Init(void);

				void AddGeometry(const string &name,
								 shared_ptr<geometry::Geometry3D> spG,
								 rendering::Material *material = nullptr,
								 bool bVisible = true);
				void AddPointCloud(const string &name,
								   shared_ptr<t::geometry::PointCloud> sTg,
								   rendering::Material *material = nullptr,
								   bool bVisible = true);
				void UpdatePointCloud(const string &name,
									  shared_ptr<t::geometry::PointCloud> sTg);
				void RemoveGeometry(const string &name);
				DrawObject GetGeometry(const string &name) const;
				vector<O3DVisualizerSelections::SelectionSet> GetSelectionSets() const;

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

				void camMove(Vector3f vM);

				UIState *getUIState(void);
				void UpdateUIstate(void);
            	void SetPointSize(int px);
            	void SetSelectedPointSize(double px);
    	        void SetLineWidth(int px);

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
				void ExportCurrentImage(const string &path);

			protected:
				void Layout(const gui::Theme &theme);
	            void InitCtrlPanel(void);

	            void UpdateTgeometry(const string &name, shared_ptr<t::geometry::PointCloud> sTg);

	            float ConvertToScaledPixels(int px);
	            void UpdateSelectableGeometry(void);

	            void NewVertexSet(void);
            	void SelectVertexSet(int i);
	            void UpdateVertexSetList(void);
            	void RemoveVertexSet(int i);
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
				UImode m_uiMode = uiMode_cam;
				string m_modelName;
				string m_areaName;
				bool m_bScanning;
				bool m_bCamAuto;
				vector<shared_ptr<Label3D>> m_vspDistLabel;

				//UI components
				Vert *m_panelCtrl;

				Button* m_btnOpenPC;
				Button* m_btnSavePC;
				Button* m_btnSaveRGB;

				Button* m_btnCamAuto;
				Button* m_btnCamAll;
				Button* m_btnCamOrigin;
				Button* m_btnCamL;
				Button* m_btnCamR;
				Button* m_btnCamF;
				Button* m_btnCamB;
				Button* m_btnCamU;
				Button* m_btnCamD;

				Slider* m_sliderVsize;
				Button* m_btnHiddenRemove;
				Button* m_btnFilterReset;
				// Slider* m_sliderORemovN;
				// Slider* m_sliderORemovD;

				Button *m_btnScanStart;
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
