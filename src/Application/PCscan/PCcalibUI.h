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

				double m_oTx = 0.0;
				double m_oTy = 0.0;
				double m_oTz = 0.0;

				double m_oRx = 0.0;
				double m_oRy = 0.0;
				double m_oRz = 0.0;

				double m_k1 = 0.0;
				double m_k2 = 0.0;
				double m_p1 = 0.0;
				double m_p2 = 0.0;
				double m_k3 = 0.0;
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

				void SetCbLoadImgs(OnCbO3DUI pCb, void *pPCV);
				void SetCbResetPC(OnCbO3DUI pCb, void *pPCV);
				void SetCbUpdateParams(OnCbO3DUI pCb, void *pPCV);

			protected:
				virtual void Layout(const gui::Theme &theme);
				void InitCtrlPanel(void);

				void OnImportParams(void);
				void OnExportParams(void);

			private:
				double m_dV;
				int m_nD;
				PCCALIB_PARAM m_calibParam;

				//UI components
				Vert *m_panelCtrl;

				TextEdit *m_pPath;

				TextEdit *m_pFx;
				TextEdit *m_pFy;
				TextEdit *m_pCx;
				TextEdit *m_pCy;

				TextEdit *m_poTx;
				TextEdit *m_poTy;
				TextEdit *m_poTz;

				TextEdit *m_poRx;
				TextEdit *m_poRy;
				TextEdit *m_poRz;

				TextEdit *m_pk1;
				TextEdit *m_pk2;
				TextEdit *m_pp1;
				TextEdit *m_pp2;
				TextEdit *m_pk3;

				//UI handler
				O3D_UI_Cb m_cbLoadImgs;
				O3D_UI_Cb m_cbResetPC;
				O3D_UI_Cb m_cbUpdateParams;
			};

		} // namespace visualizer
	}	  // namespace visualization
} // namespace open3d

#endif
