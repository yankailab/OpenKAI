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
				float m_Fx = 0.0;
				float m_Fy = 0.0;
				float m_Cx = 0.0;
				float m_Cy = 0.0;

				float m_oTx = 0.0;
				float m_oTy = 0.0;
				float m_oTz = 0.0;

				float m_oRx = 0.0;
				float m_oRy = 0.0;
				float m_oRz = 0.0;

				float m_k1 = 0.0;
				float m_k2 = 0.0;
				float m_k3 = 0.0;
				float m_k4 = 0.0;
			};

			enum MenuId
			{
				MENU_LOAD_IMGS,
				MENU_IMPORT_PARAMS,
				MENU_EXPORT_PARAMS,
				MENU_RESET_PC
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

				void SetCbLoadImgs(OnBtnClickedCb pCb, void *pPCV);
				void SetCbResetPC(OnBtnClickedCb pCb, void *pPCV);

			protected:
				virtual void Layout(const gui::Theme &theme);
				void InitCtrlPanel(void);

				void OnLoadImgs(void);
				void OnImportParams(void);
				void OnExportParams(void);

			private:
				float m_dV;
				PCCALIB_PARAM m_calibParam;

				//UI components
				Vert *m_panelCtrl;

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
				TextEdit *m_pk3;
				TextEdit *m_pk4;

				//UI handler
				O3D_UI_Cb m_cbLoadImgs;
				O3D_UI_Cb m_cbResetPC;
			};

		} // namespace visualizer
	}	  // namespace visualization
} // namespace open3d

#endif
