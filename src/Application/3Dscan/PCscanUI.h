#ifndef OpenKAI_src_Application_3Dscan__PCscanUI_H_
#define OpenKAI_src_Application_3Dscan__PCscanUI_H_

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

	            void SetMouseCameraMode(void);
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
				void OnSaveRGB(void);
				void OnSavePLY(void);
				void OnOpenPLY(void);

			private:
				shared_ptr<O3DVisualizerSelections> m_sVertex;
				UImode m_uiMode = uiMode_cam;
				string m_areaName;
				bool m_bScanning;
				bool m_bCamAuto;

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
