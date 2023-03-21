#ifndef OpenKAI_src_Application_LivoxScan_LivoxScanAutoUI_H_
#define OpenKAI_src_Application_LivoxScan_LivoxScanAutoUI_H_

#include "../../LIDAR/Livox/_Livox.h"
#include "../../UI/O3DUI.h"

namespace open3d
{
	namespace visualization
	{
		namespace visualizer
		{
			enum LivoxScanSetCmd
			{
				lvc_HL,
				lvc_HR,
				lvc_VT,
				lvc_VB,
				lvc_Rst,
			};

			struct LivoxScanSet
			{
				int m_tWaitSec = 10;
				vFloat2 m_vSvRangeH = {0.0, 1.0};
				vFloat2 m_vSvRangeV = {0.0, 1.0};

				int m_nH = 10;
				int m_nV = 10;

				LivoxScanSetCmd m_lastSet = lvc_Rst;
			};

			class _LivoxScanAutoUI : public O3DUI
			{
			public:
				_LivoxScanAutoUI(const string &title, int width, int height);
				virtual ~_LivoxScanAutoUI();

				// overloaded
				virtual void Init(void);
				virtual void camMove(Vector3f vM);
				virtual void UpdateUIstate(void);

				void SetMouseCameraMode(void);
				void SetProgressBar(float v);
				void SetIsScanning(bool b);
				void SetScanSet(const LivoxScanSet& s);
				LivoxScanSet GetScanSet(void);

				void SetCbScanReset(OnCbO3DUI pCb, void *pPCV);
				void SetCbScanSet(OnCbO3DUI pCb, void *pPCV);
				void SetCbScanStart(OnCbO3DUI pCb, void *pPCV);
				void SetCbScanStop(OnCbO3DUI pCb, void *pPCV);

				void SetCbSavePC(OnCbO3DUI pCb, void *pPCV);
				void SetCbCamSet(OnCbO3DUI pCb, void *pPCV);
				void SetCbCamCtrl(OnCbO3DUI pCb, void *pPCV);

			protected:
				virtual void Layout(const gui::LayoutContext &context);

				void InitCtrlPanel(void);
				void UpdateBtnState(void);
				void OnSavePLY(void);

			private:
				UImode m_uiMode = uiMode_cam;
				bool m_bCamAuto;
				LivoxCtrl m_livoxCtrl;
				int m_pointSize;

				LivoxScanSet m_scanSet;
				bool m_bScanning;

				// UI components
				Vert *m_panelCtrl;

				Button *m_pBtnCamAuto;
				Button *m_pBtnCamAll;
				Button *m_pBtnCamOrigin;
				Button *m_pBtnCamL;
				Button *m_pBtnCamR;
				Button *m_pBtnCamF;
				Button *m_pBtnCamB;
				Button *m_pBtnCamU;
				Button *m_pBtnCamD;

				Button *m_pBtnSavePC;
				Button *m_pBtnScanReset;
				Button *m_pBtnScanStart;
				ProgressBar *m_progScan;
				Label *m_labelProg;

				Button *m_pBtnTwaitSec;
				Button *m_pBtnTSinc;
				Button *m_pBtnTSdec;

				Button *m_pBtnNH;
				Button *m_pBtnAHL;
				Button *m_pBtnAHR;
				Button *m_pBtnNV;
				Button *m_pBtnAVT;
				Button *m_pBtnAVB;
				Button *m_pBtnNHinc;
				Button *m_pBtnNHdec;
				Button *m_pBtnAHLinc;
				Button *m_pBtnAHLdec;
				Button *m_pBtnAHRinc;
				Button *m_pBtnAHRdec;
				Button *m_pBtnNVinc;
				Button *m_pBtnNVdec;
				Button *m_pBtnAVTinc;
				Button *m_pBtnAVTdec;
				Button *m_pBtnAVBinc;
				Button *m_pBtnAVBdec;

				Button *m_pBtnPointSize;
				Button *m_pBtnMinD;
				Button *m_pBtnMaxD;

				// UI handler
				O3D_UI_Cb m_cbScanReset;
				O3D_UI_Cb m_cbScanSet;
				O3D_UI_Cb m_cbScanStart;
				O3D_UI_Cb m_cbScanStop;

				O3D_UI_Cb m_cbSavePC;
				O3D_UI_Cb m_cbCamSet;
				O3D_UI_Cb m_cbCamCtrl;
			};

		} // namespace visualizer
	}	  // namespace visualization
} // namespace open3d
#endif
