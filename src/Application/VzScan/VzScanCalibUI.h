#ifndef OpenKAI_src_Application_VzScan_VzScanCalibUI_H_
#define OpenKAI_src_Application_Vzscan_VzScanCalibUI_H_

#include "../../LIDAR/_VzensePC.h"
#include "../../UI/O3DUI.h"

namespace open3d
{
	namespace visualization
	{
		namespace visualizer
		{
			class _VzScanCalibUI : public O3DUI
			{
			public:
				_VzScanCalibUI(const string &title, int width, int height);
				virtual ~_VzScanCalibUI();

				//overloaded
				virtual void Init(void);
				virtual void camMove(Vector3f vM);
				virtual void UpdateUIstate(void);

	            void SetMouseCameraMode(void);
				void SetProgressBar(float v);
	            void SetLabelArea(const string &s);
				void SetCbScanReset(OnCbO3DUI pCb, void *pPCV);
				void SetCbScanTake(OnCbO3DUI pCb, void *pPCV);
				void SetCbSavePC(OnCbO3DUI pCb, void *pPCV);
				void SetCbCamSet(OnCbO3DUI pCb, void *pPCV);
				void SetCbCamCtrl(OnCbO3DUI pCb, void *pPCV);

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
				bool m_bCamAuto;
				VzCtrl m_camCtrl;

				//UI components
				Vert *m_panelCtrl;
				Button* m_btnSavePC;

				Button* m_btnCamAuto;
				Button* m_btnCamAll;
				Button* m_btnCamOrigin;
				Button* m_btnCamL;
				Button* m_btnCamR;
				Button* m_btnCamF;
				Button* m_btnCamB;
				Button* m_btnCamU;
				Button* m_btnCamD;

				Button *m_btnScanReset;
				Button *m_btnScanTake;
				ProgressBar *m_progScan;
				Label *m_labelProg;

				Button* m_btnFillHole;
				Button* m_btnFilSpatial;
				Button* m_btnHDR;
				
				//UI handler
				O3D_UI_Cb m_cbScanReset;
				O3D_UI_Cb m_cbScanTake;
				O3D_UI_Cb m_cbSavePC;
				O3D_UI_Cb m_cbCamSet;
				O3D_UI_Cb m_cbCamCtrl;
			};

		} // namespace visualizer
	}	  // namespace visualization
} // namespace open3d
#endif
