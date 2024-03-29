#include "VzScanUI.h"

namespace open3d
{
    namespace visualization
    {
        namespace visualizer
        {
            namespace
            {
                template <typename T>
                shared_ptr<T> GiveOwnership(T *ptr)
                {
                    return shared_ptr<T>(ptr);
                }
            }

            _VzScanUI::_VzScanUI(const string &title, int width, int height)
                : O3DUI(title, width, height)
            {
            }

            _VzScanUI::~_VzScanUI() {}

            void _VzScanUI::Init(void)
            {
                this->O3DUI::Init();

                m_bCamAuto = false;
                m_pointSize = 2;
                m_sVertex = make_shared<O3DVisualizerSelections>(*m_pScene);
                InitCtrlPanel();
                UpdateUIstate();
                SetMouseCameraMode();

                Application::GetInstance().SetMenubar(NULL);
            }

            void _VzScanUI::camMove(Vector3f vM)
            {
                auto pC = m_pScene->GetScene()->GetCamera();
                auto mm = pC->GetModelMatrix();
                vM *= m_uiState.m_sMove;
                mm.translate(vM);
                pC->SetModelMatrix(mm);

                if (m_uiMode == uiMode_pointPick)
                    UpdateSelectableGeometry();
                m_pScene->ForceRedraw();
            }

            void _VzScanUI::UpdateUIstate(void)
            {
                m_panelCtrl->SetVisible(m_uiState.m_bShowPanel);
                UpdateBtnState();

                this->O3DUI::UpdateUIstate();
            }

            void _VzScanUI::UpdateBtnState(void)
            {
                m_pBtnScanReset->SetEnabled(true);
                m_pBtnScanTake->SetEnabled(true);
                m_pBtnSavePC->SetEnabled(true);

                m_pBtnCamAuto->SetOn(m_bCamAuto);
                m_pBtnCamAuto->SetEnabled(true);
                m_pBtnCamAll->SetEnabled(!m_bCamAuto);
                m_pBtnCamOrigin->SetEnabled(!m_bCamAuto);
                m_pBtnCamL->SetEnabled(!m_bCamAuto);
                m_pBtnCamR->SetEnabled(!m_bCamAuto);
                m_pBtnCamF->SetEnabled(!m_bCamAuto);
                m_pBtnCamB->SetEnabled(!m_bCamAuto);
                m_pBtnCamU->SetEnabled(!m_bCamAuto);
                m_pBtnCamD->SetEnabled(!m_bCamAuto);

                m_pBtnPointSize->SetText(string("Point Size: " + i2str(m_pointSize)).c_str());
                m_pBtnMinD->SetText(string("Min distance: " + f2str(m_camCtrl.m_vRz.x)).c_str());
                m_pBtnMaxD->SetText(string("Max distance: " + f2str(m_camCtrl.m_vRz.y)).c_str());

                if (m_camCtrl.m_bAutoExposureToF)
                {
                    m_pBtnToFexp->SetText("ToF exposure: AUTO");
                    m_pBtnToFexp->SetOn(true);
                }
                else
                {
                    m_pBtnToFexp->SetText(string("ToF exposure: " + i2str(m_camCtrl.m_tExposureToF)).c_str());
                    m_pBtnToFexp->SetOn(false);
                }

                if (m_camCtrl.m_bAutoExposureRGB)
                    m_pBtnRGBexp->SetText("RGB exposure: AUTO");
                else
                    m_pBtnRGBexp->SetText(string("RGB exposure: " + i2str(m_camCtrl.m_tExposureRGB)).c_str());

                if (m_camCtrl.m_bFilTime)
                    m_pBtnTfilter->SetText(string("Time filter: " + i2str(m_camCtrl.m_filTime)).c_str());
                else
                    m_pBtnTfilter->SetText("Time filter: OFF");

                if (m_camCtrl.m_bFilConfidence)
                    m_pBtnCfilter->SetText(string("Confidence: " + i2str(m_camCtrl.m_filConfidence)).c_str());
                else
                    m_pBtnCfilter->SetText("Confidence: OFF");

                if (m_camCtrl.m_bFilFlyingPix)
                    m_pBtnFpFilter->SetText(string("FlyingPixel: " + i2str(m_camCtrl.m_filFlyingPix)).c_str());
                else
                    m_pBtnFpFilter->SetText("FlyingPixel: OFF");

                m_pBtnFillHole->SetEnabled(true);
                m_pBtnFilSpatial->SetEnabled(true);
                m_pBtnHDR->SetEnabled(true);
                m_pBtnFillHole->SetText(m_camCtrl.m_bFillHole ? "Fill hole ON" : "Fill hole OFF");
                m_pBtnFilSpatial->SetText(m_camCtrl.m_bSpatialFilter ? "Spatial ON" : "Spatial OFF");
                m_pBtnHDR->SetText(m_camCtrl.m_bHDR ? "HDR ON" : "HDR OFF");

                m_pScene->ForceRedraw();
            }

            void _VzScanUI::SetMouseCameraMode(void)
            {
                if (m_sVertex->IsActive() && m_sVertex->GetNumberOfSets() > 0)
                    m_sVertex->MakeInactive();

                m_pScene->SetViewControls(m_uiState.m_mouseMode);
                m_uiMode = uiMode_cam;
            }

            void _VzScanUI::SetProgressBar(float v)
            {
                m_progScan->SetValue(v);
                string s = "Memory used: " + i2str((int)(v * 100)) + "%";
                m_labelProg->SetText(s.c_str());
            }

            void _VzScanUI::SetCbScanReset(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbScanReset.add(pCb, pPCV);
            }

            void _VzScanUI::SetCbScanTake(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbScanTake.add(pCb, pPCV);
            }

            void _VzScanUI::SetCbSavePC(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbSavePC.add(pCb, pPCV);
            }

            void _VzScanUI::SetCbCamSet(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbCamSet.add(pCb, pPCV);
            }

            void _VzScanUI::SetCbCamCtrl(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbCamCtrl.add(pCb, pPCV);
            }

            void _VzScanUI::Layout(const gui::LayoutContext &context)
            {
                int settings_width = m_uiState.m_wPanel * context.theme.font_size;

                auto f = GetContentRect();
                if (m_panelCtrl->IsVisible())
                {
                    m_pScene->SetFrame(gui::Rect(f.x, f.y, f.width - settings_width, f.height));
                    m_panelCtrl->SetFrame(gui::Rect(f.GetRight() - settings_width, f.y, settings_width, f.height));
                }
                else
                {
                    m_pScene->SetFrame(f);
                }

                gui::Window::Layout(context);
            }

            void _VzScanUI::InitCtrlPanel(void)
            {
                auto em = GetTheme().font_size;
                auto half_em = int(round(0.5f * float(em)));
                auto v_spacing = int(round(0.25 * float(em)));

                m_panelCtrl = new Vert(half_em);
                AddChild(GiveOwnership(m_panelCtrl));

                Margins margins(em, 0, half_em, 0);

                // Camera
                auto panelCam = new CollapsableVert("CAM", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelCam));

                m_pBtnCamAuto = new Button("  Auto  ");
                m_pBtnCamAuto->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnCamAuto->SetOnClicked([this]()
                                            {
                                               m_bCamAuto = !m_bCamAuto;
                                               int m = m_bCamAuto ? 1 : 0;
                                               m_cbCamSet.call(&m);
                                               UpdateBtnState();
                                               m_pScene->ForceRedraw(); });

                m_pBtnCamAll = new Button("    All    ");
                m_pBtnCamAll->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnCamAll->SetOnClicked([this]()
                                           {
                                              int m = 4;
                                              m_cbCamSet.call(&m);
                                              if (m_uiMode == uiMode_pointPick)
                                                  UpdateSelectableGeometry();
                                              m_pScene->ForceRedraw(); });

                m_pBtnCamOrigin = new Button(" Origin ");
                m_pBtnCamOrigin->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnCamOrigin->SetOnClicked([this]()
                                              {
                                                 int m = 3;
                                                 m_cbCamSet.call(&m);
                                                 if (m_uiMode == uiMode_pointPick)
                                                     UpdateSelectableGeometry();
                                                 m_pScene->ForceRedraw(); });

                m_pBtnCamL = new Button(" < ");
                m_pBtnCamL->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnCamL->SetOnClicked([this]()
                                         { camMove(Vector3f(-1, 0, 0)); });

                m_pBtnCamR = new Button(" > ");
                m_pBtnCamR->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnCamR->SetOnClicked([this]()
                                         { camMove(Vector3f(1, 0, 0)); });

                m_pBtnCamF = new Button(" ^ ");
                m_pBtnCamF->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnCamF->SetOnClicked([this]()
                                         { camMove(Vector3f(0, 0, -1)); });

                m_pBtnCamB = new Button(" v ");
                m_pBtnCamB->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnCamB->SetOnClicked([this]()
                                         { camMove(Vector3f(0, 0, 1)); });

                m_pBtnCamU = new Button(" Up ");
                m_pBtnCamU->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnCamU->SetOnClicked([this]()
                                         { camMove(Vector3f(0, 1, 0)); });

                m_pBtnCamD = new Button(" Down ");
                m_pBtnCamD->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnCamD->SetOnClicked([this]()
                                         { camMove(Vector3f(0, -1, 0)); });

                auto *pN = new VGrid(3, v_spacing);
                pN->AddChild(GiveOwnership(m_pBtnCamAuto));
                pN->AddChild(GiveOwnership(m_pBtnCamAll));
                pN->AddChild(GiveOwnership(m_pBtnCamOrigin));
                pN->AddChild(GiveOwnership(m_pBtnCamD));
                pN->AddChild(GiveOwnership(m_pBtnCamF));
                pN->AddChild(GiveOwnership(m_pBtnCamU));
                pN->AddChild(GiveOwnership(m_pBtnCamL));
                pN->AddChild(GiveOwnership(m_pBtnCamB));
                pN->AddChild(GiveOwnership(m_pBtnCamR));
                panelCam->AddChild(GiveOwnership(pN));

                // Scan
                auto panelScan = new CollapsableVert("SCAN", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelScan));

                m_pBtnScanReset = new Button(" Reset ");
                m_pBtnScanReset->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnScanReset->SetOnClicked([this]()
                                              {
                                            m_cbScanReset.call();
                                            int m = m_bCamAuto ? 1 : 0;
                                            m_cbCamSet.call(&m);
                                            UpdateBtnState();
                                            PostRedraw(); });

                m_pBtnSavePC = new Button("  Save  ");
                m_pBtnSavePC->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnSavePC->SetOnClicked([this]()
                                           { OnSavePLY(); });

                m_pBtnScanTake = new Button("  Take  ");
                m_pBtnScanTake->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnScanTake->SetOnClicked([this]()
                                             { m_cbScanTake.call(); });

                auto *pG = new VGrid(3, v_spacing);
                pG->AddChild(GiveOwnership(m_pBtnScanReset));
                pG->AddChild(GiveOwnership(m_pBtnSavePC));
                pG->AddChild(GiveOwnership(m_pBtnScanTake));
                panelScan->AddChild(GiveOwnership(pG));

                m_labelProg = new Label("Memory used: 0%");
                auto pH = new Horiz(v_spacing);
                pH->AddChild(GiveOwnership(m_labelProg));
                pH->AddStretch();
                panelScan->AddChild(GiveOwnership(pH));

                m_progScan = new ProgressBar();
                m_progScan->SetValue(0.0);
                pH = new Horiz(v_spacing);
                pH->AddChild(GiveOwnership(m_progScan));
                panelScan->AddChild(GiveOwnership(pH));

                // Filter
                auto panelFilter = new CollapsableVert("FILTER", v_spacing, margins);
                panelFilter->SetIsOpen(false);
                m_panelCtrl->AddChild(GiveOwnership(panelFilter));

                int btnPadH = m_uiState.m_btnPaddingH * 1.2;
                int btnPadV = m_uiState.m_btnPaddingV / 2;

                VGrid *pGc = new VGrid(3, v_spacing);

                // Point size
                m_pBtnPointSize = new Button("Point Size: ");
                Button *pBtnPSinc = new Button(" + ");
                pBtnPSinc->SetOnClicked([this]()
                                        {
                                        m_pointSize = constrain(m_pointSize+1, 1, 10);
                                        SetPointSize(m_pointSize);
                                        UpdateBtnState();
                                        m_pScene->ForceRedraw(); });
                Button *pBtnPSdec = new Button(" - ");
                pBtnPSdec->SetOnClicked([this]()
                                        {
                                        m_pointSize = constrain(m_pointSize-1, 1, 10);
                                        SetPointSize(m_pointSize);
                                        UpdateBtnState();
                                        m_pScene->ForceRedraw(); });
                m_pBtnPointSize->SetPaddingEm(btnPadH, btnPadV);
                pBtnPSinc->SetPaddingEm(btnPadH, btnPadV);
                pBtnPSdec->SetPaddingEm(btnPadH, btnPadV);
                pGc->AddChild(GiveOwnership(m_pBtnPointSize));
                pGc->AddChild(GiveOwnership(pBtnPSinc));
                pGc->AddChild(GiveOwnership(pBtnPSdec));

                // Min distance
                m_pBtnMinD = new Button("Min distance: ");
                Button *pBtnMinDinc = new Button(" + ");
                pBtnMinDinc->SetOnClicked([this]()
                                          {
                                            m_camCtrl.m_vRz.x = constrain<float>(m_camCtrl.m_vRz.x + 0.1, 0, m_camCtrl.m_vRz.y);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });
                Button *pBtnMinDdec = new Button(" - ");
                pBtnMinDdec->SetOnClicked([this]()
                                          {
                                            m_camCtrl.m_vRz.x = constrain<float>(m_camCtrl.m_vRz.x - 0.1, 0, m_camCtrl.m_vRz.y);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });
                m_pBtnMinD->SetPaddingEm(btnPadH, btnPadV);
                pBtnMinDinc->SetPaddingEm(btnPadH, btnPadV);
                pBtnMinDdec->SetPaddingEm(btnPadH, btnPadV);
                pGc->AddChild(GiveOwnership(m_pBtnMinD));
                pGc->AddChild(GiveOwnership(pBtnMinDinc));
                pGc->AddChild(GiveOwnership(pBtnMinDdec));

                // Max distance
                m_pBtnMaxD = new Button("Max distance: ");
                Button *pBtnMaxDinc = new Button(" + ");
                pBtnMaxDinc->SetOnClicked([this]()
                                          {
                                            m_camCtrl.m_vRz.y = constrain<float>(m_camCtrl.m_vRz.y + 0.1, m_camCtrl.m_vRz.x, 10);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });
                Button *pBtnMaxDdec = new Button(" - ");
                pBtnMaxDdec->SetOnClicked([this]()
                                          {
                                            m_camCtrl.m_vRz.y = constrain<float>(m_camCtrl.m_vRz.y - 0.1, m_camCtrl.m_vRz.x, 10);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });
                m_pBtnMaxD->SetPaddingEm(btnPadH, btnPadV);
                pBtnMaxDinc->SetPaddingEm(btnPadH, btnPadV);
                pBtnMaxDdec->SetPaddingEm(btnPadH, btnPadV);
                pGc->AddChild(GiveOwnership(m_pBtnMaxD));
                pGc->AddChild(GiveOwnership(pBtnMaxDinc));
                pGc->AddChild(GiveOwnership(pBtnMaxDdec));

                // ToF exposure
                m_pBtnToFexp = new Button("ToF exposure: ");
                m_pBtnToFexp->SetOnClicked([this]()
                                           {
                                                m_camCtrl.m_bAutoExposureToF = !m_camCtrl.m_bAutoExposureToF;
                                                m_cbCamCtrl.call(&m_camCtrl);
                                                UpdateBtnState();
                                                m_pScene->ForceRedraw(); });
                Button *pBtnTEinc = new Button(" + ");
                pBtnTEinc->SetOnClicked([this]()
                                        {
                                            m_camCtrl.m_tExposureToF = constrain<int>(m_camCtrl.m_tExposureToF + 1000, 1000, 4000);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });
                Button *pBtnTEdec = new Button(" - ");
                pBtnTEdec->SetOnClicked([this]()
                                        {
                                            m_camCtrl.m_tExposureToF = constrain<int>(m_camCtrl.m_tExposureToF - 1000, 1000, 4000);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });
                m_pBtnToFexp->SetPaddingEm(btnPadH, btnPadV);
                pBtnTEinc->SetPaddingEm(btnPadH, btnPadV);
                pBtnTEdec->SetPaddingEm(btnPadH, btnPadV);
                pGc->AddChild(GiveOwnership(m_pBtnToFexp));
                pGc->AddChild(GiveOwnership(pBtnTEinc));
                pGc->AddChild(GiveOwnership(pBtnTEdec));

                // RGB exposure
                m_pBtnRGBexp = new Button("RGB exposure: ");
                m_pBtnRGBexp->SetOnClicked([this]()
                                           {
                                                m_camCtrl.m_bAutoExposureRGB = !m_camCtrl.m_bAutoExposureRGB;
                                                m_cbCamCtrl.call(&m_camCtrl);
                                                UpdateBtnState();
                                                m_pScene->ForceRedraw(); });
                Button *pBtnREinc = new Button(" + ");
                pBtnREinc->SetOnClicked([this]()
                                        {
                                            m_camCtrl.m_tExposureRGB = constrain<int>(m_camCtrl.m_tExposureRGB + 1000, 1000, 4000);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });
                Button *pBtnREdec = new Button(" - ");
                pBtnREdec->SetOnClicked([this]()
                                        {
                                            m_camCtrl.m_tExposureRGB = constrain<int>(m_camCtrl.m_tExposureRGB - 1000, 1000, 4000);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });
                m_pBtnRGBexp->SetPaddingEm(btnPadH, btnPadV);
                pBtnREinc->SetPaddingEm(btnPadH, btnPadV);
                pBtnREdec->SetPaddingEm(btnPadH, btnPadV);
                pGc->AddChild(GiveOwnership(m_pBtnRGBexp));
                pGc->AddChild(GiveOwnership(pBtnREinc));
                pGc->AddChild(GiveOwnership(pBtnREdec));

                // Time filter
                m_pBtnTfilter = new Button("Time filter: ");
                m_pBtnTfilter->SetOnClicked([this]()
                                            {
                                                m_camCtrl.m_bFilTime = !m_camCtrl.m_bFilTime;
                                                m_cbCamCtrl.call(&m_camCtrl);
                                                UpdateBtnState();
                                                m_pScene->ForceRedraw(); });
                Button *pBtnTFinc = new Button(" + ");
                pBtnTFinc->SetOnClicked([this]()
                                        {
                                            m_camCtrl.m_filTime = constrain<int>(m_camCtrl.m_filTime + 1, 0, 3);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                             UpdateBtnState();
                                           m_pScene->ForceRedraw(); });
                Button *pBtnTFdec = new Button(" - ");
                pBtnTFdec->SetOnClicked([this]()
                                        {
                                            m_camCtrl.m_filTime = constrain<int>(m_camCtrl.m_filTime - 1, 0, 3);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });
                m_pBtnTfilter->SetPaddingEm(btnPadH, btnPadV);
                pBtnTFinc->SetPaddingEm(btnPadH, btnPadV);
                pBtnTFdec->SetPaddingEm(btnPadH, btnPadV);
                pGc->AddChild(GiveOwnership(m_pBtnTfilter));
                pGc->AddChild(GiveOwnership(pBtnTFinc));
                pGc->AddChild(GiveOwnership(pBtnTFdec));

                // Confidence filter
                m_pBtnCfilter = new Button("Confidence filter: ");
                m_pBtnCfilter->SetOnClicked([this]()
                                            {
                                                m_camCtrl.m_bFilConfidence = !m_camCtrl.m_bFilConfidence;
                                                m_cbCamCtrl.call(&m_camCtrl);
                                                UpdateBtnState();
                                                m_pScene->ForceRedraw(); });
                Button *pBtnCFinc = new Button(" + ");
                pBtnCFinc->SetOnClicked([this]()
                                        {
                                            m_camCtrl.m_filConfidence = constrain<int>(m_camCtrl.m_filConfidence + 1, 0, 100);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });
                Button *pBtnCFdec = new Button(" - ");
                pBtnCFdec->SetOnClicked([this]()
                                        {
                                            m_camCtrl.m_filConfidence = constrain<int>(m_camCtrl.m_filConfidence - 1, 0, 100);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });
                m_pBtnCfilter->SetPaddingEm(btnPadH, btnPadV);
                pBtnCFinc->SetPaddingEm(btnPadH, btnPadV);
                pBtnCFdec->SetPaddingEm(btnPadH, btnPadV);
                pGc->AddChild(GiveOwnership(m_pBtnCfilter));
                pGc->AddChild(GiveOwnership(pBtnCFinc));
                pGc->AddChild(GiveOwnership(pBtnCFdec));

                // Flying Pixel
                m_pBtnFpFilter = new Button("FlyingPixel: ");
                m_pBtnFpFilter->SetOnClicked([this]()
                                             {
                                                m_camCtrl.m_bFilFlyingPix = !m_camCtrl.m_bFilFlyingPix;
                                                m_cbCamCtrl.call(&m_camCtrl);
                                                UpdateBtnState();
                                                m_pScene->ForceRedraw(); });
                Button *pBtnFPinc = new Button(" + ");
                pBtnFPinc->SetOnClicked([this]()
                                        {
                                            m_camCtrl.m_filFlyingPix = constrain<int>(m_camCtrl.m_filFlyingPix + 1, 0, 49);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });
                Button *pBtnFPdec = new Button(" - ");
                pBtnFPdec->SetOnClicked([this]()
                                        {
                                            m_camCtrl.m_filFlyingPix = constrain<int>(m_camCtrl.m_filFlyingPix - 1, 0, 49);
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });
                m_pBtnFpFilter->SetPaddingEm(btnPadH, btnPadV);
                pBtnFPinc->SetPaddingEm(btnPadH, btnPadV);
                pBtnFPdec->SetPaddingEm(btnPadH, btnPadV);
                pGc->AddChild(GiveOwnership(m_pBtnFpFilter));
                pGc->AddChild(GiveOwnership(pBtnFPinc));
                pGc->AddChild(GiveOwnership(pBtnFPdec));

                panelFilter->AddChild(GiveOwnership(pGc));

                // Options
                btnPadH = m_uiState.m_btnPaddingH * 0.8;
                btnPadV = m_uiState.m_btnPaddingV;
                m_pBtnFillHole = new Button("Fill hole");
                m_pBtnFillHole->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnFillHole->SetOnClicked([this]()
                                             {
                                                m_camCtrl.m_bFillHole = !m_camCtrl.m_bFillHole;
                                                m_cbCamCtrl.call(&m_camCtrl);
                                                UpdateBtnState();
                                                m_pScene->ForceRedraw(); });

                m_pBtnFilSpatial = new Button("Spatial filter");
                m_pBtnFilSpatial->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnFilSpatial->SetOnClicked([this]()
                                               {
                                                m_camCtrl.m_bSpatialFilter = !m_camCtrl.m_bSpatialFilter;
                                                m_cbCamCtrl.call(&m_camCtrl);
                                                UpdateBtnState();
                                                m_pScene->ForceRedraw(); });

                m_pBtnHDR = new Button(" HDR ");
                m_pBtnHDR->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnHDR->SetOnClicked([this]()
                                        {
                                            m_camCtrl.m_bHDR = !m_camCtrl.m_bHDR;
                                            m_cbCamCtrl.call(&m_camCtrl);
                                            UpdateBtnState();
                                            m_pScene->ForceRedraw(); });

                VGrid *pGo = new VGrid(3, v_spacing);
                pGo->AddChild(GiveOwnership(m_pBtnFillHole));
                pGo->AddChild(GiveOwnership(m_pBtnFilSpatial));
                pGo->AddChild(GiveOwnership(m_pBtnHDR));
                panelFilter->AddChild(GiveOwnership(pGo));
            }

            void _VzScanUI::UpdateSelectableGeometry(void)
            {
                vector<SceneWidget::PickableGeometry> pickable;
                for (auto &o : m_vObject)
                {
                    if (!o.m_bVisible)
                        continue;

                    pickable.emplace_back(o.m_name, o.m_sGeometry.get(), o.m_sTgeometry.get());
                }
                m_sVertex->SetSelectableGeometry(pickable);
            }

            void _VzScanUI::OnSaveRGB(void)
            {
                string fName = getBaseDirSave();
                if (fName.empty())
                {
                    ShowMsg("File Save", "Insert USB memory", true);
                    return;
                }

                fName += tFormat() + ".png";
                this->ExportCurrentImage(fName.c_str());
                ShowMsg("File Save", "Saved image to USB memory", true);

                // auto dlg = make_shared<gui::FileDialog>(
                //     gui::FileDialog::Mode::SAVE, "Save File", this->GetTheme());
                // dlg->AddFilter(".png", "PNG images (.png)");
                // dlg->AddFilter("", "All files");
                // dlg->SetOnCancel([this]() { this->CloseDialog(); });
                // dlg->SetOnDone([this](const char *path) {
                //     this->CloseDialog();
                //     this->ExportCurrentImage(path);
                // });
                // ShowDialog(dlg);
            }

            void _VzScanUI::OnSavePLY(void)
            {
                string fName = getBaseDirSave();
                if (fName.empty())
                {
                    ShowMsg("FILE", "Insert USB memory", true);
                    return;
                }

                fName += tFormat();
                string imgName = fName + ".png";
                this->ExportCurrentImage(imgName.c_str());
                //                string plyName = fName + ".ply";
                m_cbSavePC.call(&fName);

                // auto dlg = make_shared<gui::FileDialog>(
                //     gui::FileDialog::Mode::SAVE, "Save File", this->GetTheme());
                // dlg->AddFilter(".ply", "Point Cloud Files (.ply)");
                // dlg->AddFilter("", "All files");
                // dlg->SetOnCancel([this]() { this->CloseDialog(); });
                // dlg->SetOnDone([this](const char *path) {
                //     this->CloseDialog();

                //     ShowMsg("Save", "Saving .Ply file");
                //     io::WritePointCloudOption par;
                //     par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
                //     par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

                //     shared_ptr<t::geometry::PointCloud> spTpc = GetGeometry(m_modelName).m_sTgeometry;
                //     io::WritePointCloudToPLY(path, spTpc->ToLegacyPointCloud(), par);
                //     CloseMsg();
                // });
                // ShowDialog(dlg);
            }

            void _VzScanUI::OnOpenPLY(void)
            {
                auto dlg = make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::OPEN, "Save File", this->GetTheme());
                dlg->AddFilter(".ply", "Point Cloud Files (.ply)");
                dlg->AddFilter("", "All files");
                dlg->SetOnCancel([this]()
                                 { this->CloseDialog(); });
                dlg->SetOnDone([this](const char *path)
                               {
                                   this->CloseDialog();
                                   // this->m_cbOpenPC.call((void *)path);
                               });
                ShowDialog(dlg);
            }

        } // namespace visualizer
    }     // namespace visualization
} // namespace open3d
