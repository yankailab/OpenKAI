#pragma once
#ifdef USE_OPEN3D
#include "../../UI/O3DUI.h"

namespace open3d
{
	namespace visualization
	{
		namespace visualizer
		{
			struct PCCALIB_PARAM
			{
				double m_Fx = 0.0;
				double m_Fy = 0.0;
				double m_Cx = 0.0;
				double m_Cy = 0.0;

				double m_k1 = 0.0;
				double m_k2 = 0.0;
				double m_p1 = 0.0;
				double m_p2 = 0.0;
				double m_k3 = 0.0;

				double m_coTx = 0.0;
				double m_coTy = 0.0;
				double m_coTz = 0.0;
				double m_coRx = 0.0;
				double m_coRy = 0.0;
				double m_coRz = 0.0;

				double m_poTx = 0.0;
				double m_poTy = 0.0;
				double m_poTz = 0.0;
				double m_poRx = 0.0;
				double m_poRy = 0.0;
				double m_poRz = 0.0;
			};

			enum MenuId
			{
				MENU_IMPORT_PARAMS,
				MENU_EXPORT_PARAMS,
			};

			class PCcalibUI : public O3DUI
			{
			public:
				PCcalibUI(const string &title, int width, int height);
				virtual ~PCcalibUI();

				//overloaded
				virtual void Init(void);
				virtual void UpdateUIstate(void);

				PCCALIB_PARAM *GetCalibParams(void);
				void UpdateCalibParams(void);

				void SetCbScan(OnCbO3DUI pCb, void *pPCV);
				void SetCbResetPC(OnCbO3DUI pCb, void *pPCV);
				void SetCbRefreshCol(OnCbO3DUI pCb, void *pPCV);
				void SetCbLoadImgs(OnCbO3DUI pCb, void *pPCV);
				void SetCbUpdateParams(OnCbO3DUI pCb, void *pPCV);
				void SetCbImportParams(OnCbO3DUI pCb, void *pPCV);
				void SetCbExportParams(OnCbO3DUI pCb, void *pPCV);

			protected:
				virtual void Layout(const gui::LayoutContext &context);
				void InitCtrlPanel(void);

				void OnImportParams(void);
				void OnExportParams(void);

			private:
				double m_dV;
				int m_nD;
				PCCALIB_PARAM m_cParam;
				bool m_bScanning;

				//UI components
				Vert *m_panelCtrl;
				Button *m_btnScan;
				TextEdit *m_pPath;

				TextEdit *m_pFx;
				TextEdit *m_pFy;
				TextEdit *m_pCx;
				TextEdit *m_pCy;

				TextEdit *m_pk1;
				TextEdit *m_pk2;
				TextEdit *m_pp1;
				TextEdit *m_pp2;
				TextEdit *m_pk3;

				// RGB offset
				TextEdit *m_pCoTx;
				TextEdit *m_pCoTy;
				TextEdit *m_pCoTz;
				TextEdit *m_pCoRx;
				TextEdit *m_pCoRy;
				TextEdit *m_pCoRz;

				// SLAM offset
				TextEdit *m_pPoTx;
				TextEdit *m_pPoTy;
				TextEdit *m_pPoTz;
				TextEdit *m_pPoRx;
				TextEdit *m_pPoRy;
				TextEdit *m_pPoRz;

				//UI handler
				O3D_UI_Cb m_cbScan;
				O3D_UI_Cb m_cbResetPC;
				O3D_UI_Cb m_cbRefreshCol;
				O3D_UI_Cb m_cbLoadImgs;
				O3D_UI_Cb m_cbUpdateParams;
				O3D_UI_Cb m_cbImportParams;
				O3D_UI_Cb m_cbExportParams;
			};

		} // namespace visualizer
	}	  // namespace visualization
} // namespace open3d

#endif
