#pragma once
#ifdef USE_OPEN3D
#include "../../UI/O3DUI.h"

namespace open3d
{
	namespace visualization
	{
		namespace visualizer
		{
			class PCselectUI : public O3DUI
			{
			public:
				PCselectUI(const string &title, int width, int height);
				virtual ~PCselectUI();

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
				void SetCbScan(OnCbO3DUI pCb, void *pPCV);
				void SetCbOpenPC(OnCbO3DUI pCb, void *pPCV);
				void SetCbCamSet(OnCbO3DUI pCb, void *pPCV);
				void SetCbVoxelDown(OnCbO3DUI pCb, void *pPCV);
				void SetCbHiddenRemove(OnCbO3DUI pCb, void *pPCV);
				void SetCbResetPC(OnCbO3DUI pCb, void *pPCV);

			protected:
				virtual void Layout(const gui::LayoutContext &context);
	            
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
				Button *m_btnScan;
				ProgressBar *m_progScan;
				Label *m_labelProg;

				Button *m_btnNewVertexSet;
				Button *m_btnDeleteVertexSet;
				ListView *m_listVertexSet;
				Label *m_labelArea;

				//UI handler
				O3D_UI_Cb m_cbScan;
				O3D_UI_Cb m_cbOpenPC;
				O3D_UI_Cb m_cbCamSet;
				O3D_UI_Cb m_cbVoxelDown;
				O3D_UI_Cb m_cbHiddenRemove;
				O3D_UI_Cb m_cbResetPC;
			};

		} // namespace visualizer
	}	  // namespace visualization
} // namespace open3d

#endif
