#pragma once
#ifdef USE_OPEN3D
#include "../../UI/O3DUI.h"

namespace open3d
{
	namespace visualization
	{
		namespace visualizer
		{
			class PCcalibUI : public O3DUI
			{
			public:
				PCcalibUI(const string &title, int width, int height);
				virtual ~PCcalibUI();

				//overloaded
				virtual void Init(void);
				virtual void UpdateUIstate(void);

				void SetCbBtnCalib(OnBtnClickedCb pCb, void *pPCV);

			protected:
				virtual void Layout(const gui::Theme &theme);
	            
				void InitCtrlPanel(void);
				void UpdateBtnState(void);

				void OnSaveData(void);

			private:
				float m_dV;
				float m_Fx;

				//UI components
				Vert *m_panelCtrl;

				TextEdit* m_pFx;
				TextEdit* m_pFy;
				TextEdit* m_pCx;
				TextEdit* m_pCy;


				Button* m_btnOpenPC;
				Button* m_btnSavePC;
				Button* m_btnSaveRGB;
				Button* m_btnCamAuto;

				Slider* m_sliderVsize;
				Button* m_btnHiddenRemove;
				Button* m_btnFilterReset;
				Button *m_btnScanStart;
				ProgressBar *m_progScan;

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
