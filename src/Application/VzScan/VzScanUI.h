#ifndef OpenKAI_src_Application_VzScan_VzScanUI_H_
#define OpenKAI_src_Application_Vzscan_VzScanUI_H_

#include "../../LIDAR/_VzensePC.h"
#include "../../UI/O3DUI.h"

namespace open3d
{
	namespace visualization
	{
		namespace visualizer
		{
			class _VzScanUI : public O3DUI
			{
			public:
				_VzScanUI(const string &title, int width, int height);
				virtual ~_VzScanUI();

				// overloaded
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
				int m_pointSize;

				// UI components
				Vert *m_panelCtrl;
				Button *m_pBtnSavePC;

				Button *m_pBtnCamAuto;
				Button *m_pBtnCamAll;
				Button *m_pBtnCamOrigin;
				Button *m_pBtnCamL;
				Button *m_pBtnCamR;
				Button *m_pBtnCamF;
				Button *m_pBtnCamB;
				Button *m_pBtnCamU;
				Button *m_pBtnCamD;

				Button *m_pBtnScanReset;
				Button *m_pBtnScanTake;
				ProgressBar *m_progScan;
				Label *m_labelProg;

				Button *m_pBtnPointSize;
				Button *m_pBtnMinD;
				Button *m_pBtnMaxD;
				Button *m_pBtnToFexp;
				Button *m_pBtnRGBexp;
				Button *m_pBtnTfilter;
				Button *m_pBtnCfilter;
				Button *m_pBtnFpFilter;

				Button *m_pBtnFillHole;
				Button *m_pBtnFilSpatial;
				Button *m_pBtnHDR;

				// UI handler
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
