#pragma once
#ifdef USE_OPEN3D
#include "../../UI/O3DUI.h"

namespace open3d
{
	namespace visualization
	{
		namespace visualizer
		{
			class PCscanUI : public O3DUI
			{
			public:
				PCscanUI(const string &title, int width, int height);
				virtual ~PCscanUI();

				//overloaded
				virtual void Init(void);
				virtual void camMove(Vector3f vM);
				virtual void UpdateUIstate(void);

            	void SetSelectedPointSize(double px);
            	void UpdateSelectedPointSize(void);

	            void SetMouseCameraMode(void);
	            void SetMousePickingMode(void);
				vector<O3DVisualizerSelections::SelectionSet> GetSelectionSets() const;

				void SetProgressBar(float v);
	            void SetLabelArea(const string &s);
				void SetCbBtnScan(OnCbO3DUI pCb, void *pPCV);
				void SetCbBtnOpenPC(OnCbO3DUI pCb, void *pPCV);
				void SetCbBtnCamSet(OnCbO3DUI pCb, void *pPCV);
				void SetCbVoxelDown(OnCbO3DUI pCb, void *pPCV);
				void SetCbBtnHiddenRemove(OnCbO3DUI pCb, void *pPCV);
				void SetCbBtnResetPC(OnCbO3DUI pCb, void *pPCV);

			protected:
				virtual void Layout(const gui::Theme &theme);
	            
				void InitCtrlPanel(void);
				void UpdateBtnState(void);
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
				shared_ptr<O3DVisualizerSelections> m_sVertex;
				UImode m_uiMode = uiMode_cam;
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
