#include "LivoxScanAutoUI.h"

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

            _LivoxScanAutoUI::_LivoxScanAutoUI(const string &title, int width, int height)
                : O3DUI(title, width, height)
            {
            }

            _LivoxScanAutoUI::~_LivoxScanAutoUI() {}

            void _LivoxScanAutoUI::Init(void)
            {
                this->O3DUI::Init();

                m_bCamAuto = false;
                m_pointSize = 2;
                m_bScanning = false;
                InitCtrlPanel();
                UpdateUIstate();
                SetMouseCameraMode();

                Application::GetInstance().SetMenubar(NULL);
            }

            void _LivoxScanAutoUI::camMove(Vector3f vM)
            {
                auto pC = m_pScene->GetScene()->GetCamera();
                auto mm = pC->GetModelMatrix();
                vM *= m_uiState.m_sMove;
                mm.translate(vM);
                pC->SetModelMatrix(mm);

                m_pScene->ForceRedraw();
            }

            void _LivoxScanAutoUI::UpdateUIstate(void)
            {
                m_panelCtrl->SetVisible(m_uiState.m_bShowPanel);
                UpdateBtnState();

                this->O3DUI::UpdateUIstate();
            }

            void _LivoxScanAutoUI::UpdateBtnState(void)
            {
                m_pBtnScanReset->SetEnabled(!m_bScanning);
                m_pBtnScanStart->SetEnabled(true);
                m_pBtnScanStart->SetText(m_bScanning ? "Stop" : "Start");
                m_pBtnSavePC->SetEnabled(!m_bScanning);

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

                m_pBtnNH->SetEnabled(!m_bScanning);
                m_pBtnNHinc->SetEnabled(!m_bScanning);
                m_pBtnNHdec->SetEnabled(!m_bScanning);
                m_pBtnAHL->SetEnabled(!m_bScanning);
                m_pBtnAHLinc->SetEnabled(!m_bScanning);
                m_pBtnAHLdec->SetEnabled(!m_bScanning);
                m_pBtnAHR->SetEnabled(!m_bScanning);
                m_pBtnAHRinc->SetEnabled(!m_bScanning);
                m_pBtnAHRdec->SetEnabled(!m_bScanning);
                m_pBtnNV->SetEnabled(!m_bScanning);
                m_pBtnNVinc->SetEnabled(!m_bScanning);
                m_pBtnNVdec->SetEnabled(!m_bScanning);
                m_pBtnAVT->SetEnabled(!m_bScanning);
                m_pBtnAVTinc->SetEnabled(!m_bScanning);
                m_pBtnAVTdec->SetEnabled(!m_bScanning);
                m_pBtnAVB->SetEnabled(!m_bScanning);
                m_pBtnAVBinc->SetEnabled(!m_bScanning);
                m_pBtnAVBdec->SetEnabled(!m_bScanning);

                m_pBtnTwaitSec->SetText(string("Scan wait (sec): " + i2str(m_scanSet.m_tWaitSec)).c_str());
                m_pBtnNH->SetText(string("Horizontal Number: " + i2str(m_scanSet.m_nH)).c_str());
                m_pBtnAHL->SetText(string("Horizontal From: " + f2str(m_scanSet.m_vSvRangeH.x)).c_str());
                m_pBtnAHR->SetText(string("Horizontal To: " + f2str(m_scanSet.m_vSvRangeH.y)).c_str());
                m_pBtnNV->SetText(string("Vertical Number: " + i2str(m_scanSet.m_nV)).c_str());
                m_pBtnAVT->SetText(string("Vertical From: " + f2str(m_scanSet.m_vSvRangeV.x)).c_str());
                m_pBtnAVB->SetText(string("Vertical To: " + f2str(m_scanSet.m_vSvRangeV.y)).c_str());

                m_pBtnPointSize->SetText(string("Point Size: " + i2str(m_pointSize)).c_str());
                m_pBtnMinD->SetText(string("Min distance: " + f2str(m_livoxCtrl.m_vRz.x)).c_str());
                m_pBtnMaxD->SetText(string("Max distance: " + f2str(m_livoxCtrl.m_vRz.y)).c_str());

                m_pScene->ForceRedraw();
                PostRedraw();
            }

            void _LivoxScanAutoUI::SetMouseCameraMode(void)
            {
                m_pScene->SetViewControls(m_uiState.m_mouseMode);
                m_uiMode = uiMode_cam;
            }

            void _LivoxScanAutoUI::SetProgressBar(float v)
            {
                m_progScan->SetValue(v);
                string s = "Memory used: " + i2str((int)(v * 100)) + "%";
                m_labelProg->SetText(s.c_str());
            }

			void _LivoxScanAutoUI::SetIsScanning(bool b)
            {
                m_bScanning = b;
            }

			void _LivoxScanAutoUI::SetScanSet(const LivoxScanSet& s)
            {
                m_scanSet = s;
            }

			LivoxScanSet _LivoxScanAutoUI::GetScanSet(void)
            {
                return m_scanSet;
            }

            void _LivoxScanAutoUI::SetCbScanSet(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbScanSet.add(pCb, pPCV);
            }

            void _LivoxScanAutoUI::SetCbScanReset(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbScanReset.add(pCb, pPCV);
            }

            void _LivoxScanAutoUI::SetCbScanStart(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbScanStart.add(pCb, pPCV);
            }

            void _LivoxScanAutoUI::SetCbScanStop(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbScanStop.add(pCb, pPCV);
            }

            void _LivoxScanAutoUI::SetCbSavePC(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbSavePC.add(pCb, pPCV);
            }

            void _LivoxScanAutoUI::SetCbCamSet(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbCamSet.add(pCb, pPCV);
            }

            void _LivoxScanAutoUI::SetCbCamCtrl(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbCamCtrl.add(pCb, pPCV);
            }

            void _LivoxScanAutoUI::Layout(const gui::LayoutContext &context)
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

            void _LivoxScanAutoUI::InitCtrlPanel(void)
            {
                auto em = GetTheme().font_size;
                auto half_em = int(round(0.5f * float(em)));
                auto v_spacing = int(round(0.25 * float(em)));

                m_panelCtrl = new Vert(half_em);
                AddChild(GiveOwnership(m_panelCtrl));

                Margins margins(em, 0, half_em, 0);

                //
                // Camera
                //
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
                                              m_pScene->ForceRedraw(); });

                m_pBtnCamOrigin = new Button(" Origin ");
                m_pBtnCamOrigin->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnCamOrigin->SetOnClicked([this]()
                                              {
                                                 int m = 3;
                                                 m_cbCamSet.call(&m);
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

                auto *pGcam = new VGrid(3, v_spacing);
                pGcam->AddChild(GiveOwnership(m_pBtnCamAuto));
                pGcam->AddChild(GiveOwnership(m_pBtnCamAll));
                pGcam->AddChild(GiveOwnership(m_pBtnCamOrigin));
                pGcam->AddChild(GiveOwnership(m_pBtnCamD));
                pGcam->AddChild(GiveOwnership(m_pBtnCamF));
                pGcam->AddChild(GiveOwnership(m_pBtnCamU));
                pGcam->AddChild(GiveOwnership(m_pBtnCamL));
                pGcam->AddChild(GiveOwnership(m_pBtnCamB));
                pGcam->AddChild(GiveOwnership(m_pBtnCamR));
                panelCam->AddChild(GiveOwnership(pGcam));

                //
                // Scan
                //
                auto panelScan = new CollapsableVert("SCAN", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelScan));

                m_pBtnScanReset = new Button(" Reset ");
                m_pBtnScanReset->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnScanReset->SetOnClicked([this]()
                                              {
                                                m_cbScanReset.call();
                                                int m = m_bCamAuto ? 1 : 0;
                                                m_cbCamSet.call(&m);
                                                UpdateBtnState(); });

                m_pBtnSavePC = new Button("  Save  ");
                m_pBtnSavePC->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnSavePC->SetOnClicked([this]()
                                           { OnSavePLY(); });

                m_pBtnScanStart = new Button("  Start  ");
                m_pBtnScanStart->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_pBtnScanStart->SetOnClicked([this]()
                                              {
                                                  if (m_bScanning)
                                                  {
                                                    m_cbScanStop.call();
                                                    m_bScanning = false;
                                                    UpdateBtnState();
                                                  }
                                                  else
                                                  {
                                                    m_cbScanStart.call();
                                                    m_bScanning = true;
                                                    UpdateBtnState();
                                                  } });

                auto *pGscan = new VGrid(3, v_spacing);
                pGscan->AddChild(GiveOwnership(m_pBtnScanReset));
                pGscan->AddChild(GiveOwnership(m_pBtnSavePC));
                pGscan->AddChild(GiveOwnership(m_pBtnScanStart));
                panelScan->AddChild(GiveOwnership(pGscan));

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

                //
                // Scan settings
                //
                auto panelSetting = new CollapsableVert("SETTINGS", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelSetting));

                int btnPadH = m_uiState.m_btnPaddingH * 1.2;
                int btnPadV = m_uiState.m_btnPaddingV / 2;

                VGrid *pGs = new VGrid(3, v_spacing);

                // take wait time
                m_pBtnTwaitSec = new Button("Wait time:");
                m_pBtnTSinc = new Button(" + ");
                m_pBtnTSinc->SetOnClicked([this]()
                                        {
                                            m_scanSet.m_tWaitSec = constrain(m_scanSet.m_tWaitSec+1, 1,200);
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnTSdec = new Button(" - ");
                m_pBtnTSdec->SetOnClicked([this]()
                                        {
                                            m_scanSet.m_tWaitSec = constrain(m_scanSet.m_tWaitSec-1, 1,200);
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnTwaitSec->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnTSinc->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnTSdec->SetPaddingEm(btnPadH, btnPadV);
                pGs->AddChild(GiveOwnership(m_pBtnTwaitSec));
                pGs->AddChild(GiveOwnership(m_pBtnTSinc));
                pGs->AddChild(GiveOwnership(m_pBtnTSdec));

                // Horizontal number
                m_pBtnNH = new Button("Horizontal Number:");
                m_pBtnNHinc = new Button(" + ");
                m_pBtnNHinc->SetOnClicked([this]()
                                        {
                                            m_scanSet.m_nH = constrain(m_scanSet.m_nH+1, 1,100);
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnNHdec = new Button(" - ");
                m_pBtnNHdec->SetOnClicked([this]()
                                        {
                                            m_scanSet.m_nH = constrain(m_scanSet.m_nH-1, 1,100);
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnNH->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnNHinc->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnNHdec->SetPaddingEm(btnPadH, btnPadV);
                pGs->AddChild(GiveOwnership(m_pBtnNH));
                pGs->AddChild(GiveOwnership(m_pBtnNHinc));
                pGs->AddChild(GiveOwnership(m_pBtnNHdec));

                // Horizontal From
                m_pBtnAHL = new Button("Horizontal From:");
                m_pBtnAHL->SetOnClicked([this]()
                                        {
                                            m_scanSet.m_lastSet = lvc_HL;
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnAHLinc = new Button(" + ");
                m_pBtnAHLinc->SetOnClicked([this]()
                                         {
                                            m_scanSet.m_vSvRangeH.x = constrain<float>(m_scanSet.m_vSvRangeH.x + 0.05, 0,1);
                                            m_scanSet.m_lastSet = lvc_HL;
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnAHLdec = new Button(" - ");
                m_pBtnAHLdec->SetOnClicked([this]()
                                         {
                                            m_scanSet.m_vSvRangeH.x = constrain<float>(m_scanSet.m_vSvRangeH.x - 0.05, 0,1);
                                            m_scanSet.m_lastSet = lvc_HL;
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnAHL->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnAHLinc->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnAHLdec->SetPaddingEm(btnPadH, btnPadV);
                pGs->AddChild(GiveOwnership(m_pBtnAHL));
                pGs->AddChild(GiveOwnership(m_pBtnAHLinc));
                pGs->AddChild(GiveOwnership(m_pBtnAHLdec));

                // Horizontal To
                m_pBtnAHR = new Button("Horizontal To:");
                m_pBtnAHR->SetOnClicked([this]()
                                        {
                                            m_scanSet.m_lastSet = lvc_HR;
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnAHRinc = new Button(" + ");
                m_pBtnAHRinc->SetOnClicked([this]()
                                         {
                                            m_scanSet.m_vSvRangeH.y = constrain<float>(m_scanSet.m_vSvRangeH.y + 0.05, 0,1);
                                            m_scanSet.m_lastSet = lvc_HR;
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnAHRdec = new Button(" - ");
                m_pBtnAHRdec->SetOnClicked([this]()
                                         {
                                            m_scanSet.m_vSvRangeH.y = constrain<float>(m_scanSet.m_vSvRangeH.y - 0.05, 0,1);
                                            m_scanSet.m_lastSet = lvc_HR;
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnAHR->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnAHRinc->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnAHRdec->SetPaddingEm(btnPadH, btnPadV);
                pGs->AddChild(GiveOwnership(m_pBtnAHR));
                pGs->AddChild(GiveOwnership(m_pBtnAHRinc));
                pGs->AddChild(GiveOwnership(m_pBtnAHRdec));

                // Vertical number
                m_pBtnNV = new Button("Vertical Number:");
                m_pBtnNVinc = new Button(" + ");
                m_pBtnNVinc->SetOnClicked([this]()
                                        {
                                            m_scanSet.m_nV = constrain<float>(m_scanSet.m_nV+1, 1,100);
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnNVdec = new Button(" - ");
                m_pBtnNVdec->SetOnClicked([this]()
                                        {
                                            m_scanSet.m_nV = constrain<float>(m_scanSet.m_nV-1, 1,100);
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnNV->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnNVinc->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnNVdec->SetPaddingEm(btnPadH, btnPadV);
                pGs->AddChild(GiveOwnership(m_pBtnNV));
                pGs->AddChild(GiveOwnership(m_pBtnNVinc));
                pGs->AddChild(GiveOwnership(m_pBtnNVdec));

                // Vertical From
                m_pBtnAVT = new Button("Vertical From:");
                m_pBtnAVT->SetOnClicked([this]()
                                        {
                                            m_scanSet.m_lastSet = lvc_VT;
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnAVTinc = new Button(" + ");
                m_pBtnAVTinc->SetOnClicked([this]()
                                         {
                                            m_scanSet.m_vSvRangeV.x = constrain<float>(m_scanSet.m_vSvRangeV.x + 0.05, 0,1);
                                            m_scanSet.m_lastSet = lvc_VT;
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnAVTdec = new Button(" - ");
                m_pBtnAVTdec->SetOnClicked([this]()
                                         {
                                            m_scanSet.m_vSvRangeV.x = constrain<float>(m_scanSet.m_vSvRangeV.x - 0.05, 0,1);
                                            m_scanSet.m_lastSet = lvc_VT;
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnAVT->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnAVTinc->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnAVTdec->SetPaddingEm(btnPadH, btnPadV);
                pGs->AddChild(GiveOwnership(m_pBtnAVT));
                pGs->AddChild(GiveOwnership(m_pBtnAVTinc));
                pGs->AddChild(GiveOwnership(m_pBtnAVTdec));

                // Vertical To
                m_pBtnAVB = new Button("Vertical To:");
                m_pBtnAVB->SetOnClicked([this]()
                                        {
                                            m_scanSet.m_lastSet = lvc_VB;
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnAVBinc = new Button(" + ");
                m_pBtnAVBinc->SetOnClicked([this]()
                                         {
                                            m_scanSet.m_vSvRangeV.y = constrain<float>(m_scanSet.m_vSvRangeV.y + 0.05, 0,1);
                                            m_scanSet.m_lastSet = lvc_VB;
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnAVBdec = new Button(" - ");
                m_pBtnAVBdec->SetOnClicked([this]()
                                         {
                                            m_scanSet.m_vSvRangeV.y = constrain<float>(m_scanSet.m_vSvRangeV.y - 0.05, 0,1);
                                            m_scanSet.m_lastSet = lvc_VB;
                                            m_cbScanSet.call();
                                            UpdateBtnState(); });
                m_pBtnAVB->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnAVBinc->SetPaddingEm(btnPadH, btnPadV);
                m_pBtnAVBdec->SetPaddingEm(btnPadH, btnPadV);
                pGs->AddChild(GiveOwnership(m_pBtnAVB));
                pGs->AddChild(GiveOwnership(m_pBtnAVBinc));
                pGs->AddChild(GiveOwnership(m_pBtnAVBdec));

                panelSetting->AddChild(GiveOwnership(pGs));

                //
                // Filter
                //
                auto panelFilter = new CollapsableVert("FILTER", v_spacing, margins);
                panelFilter->SetIsOpen(false);
                m_panelCtrl->AddChild(GiveOwnership(panelFilter));

                btnPadH = m_uiState.m_btnPaddingH * 1.2;
                btnPadV = m_uiState.m_btnPaddingV / 2;

                VGrid *pGc = new VGrid(3, v_spacing);

                // Point size
                m_pBtnPointSize = new Button("Point Size: ");
                Button *pBtnPSinc = new Button(" + ");
                pBtnPSinc->SetOnClicked([this]()
                                        {
                                        m_pointSize = constrain(m_pointSize+1, 1, 10);
                                        SetPointSize(m_pointSize);
                                        UpdateBtnState(); });
                Button *pBtnPSdec = new Button(" - ");
                pBtnPSdec->SetOnClicked([this]()
                                        {
                                        m_pointSize = constrain(m_pointSize-1, 1, 10);
                                        SetPointSize(m_pointSize);
                                        UpdateBtnState(); });
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
                                            m_livoxCtrl.m_vRz.x = constrain<float>(m_livoxCtrl.m_vRz.x + 0.1, 0, m_livoxCtrl.m_vRz.y);
                                            m_cbCamCtrl.call(&m_livoxCtrl);
                                            UpdateBtnState(); });
                Button *pBtnMinDdec = new Button(" - ");
                pBtnMinDdec->SetOnClicked([this]()
                                          {
                                            m_livoxCtrl.m_vRz.x = constrain<float>(m_livoxCtrl.m_vRz.x - 0.1, 0, m_livoxCtrl.m_vRz.y);
                                            m_cbCamCtrl.call(&m_livoxCtrl);
                                            UpdateBtnState(); });
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
                                            m_livoxCtrl.m_vRz.y = constrain<float>(m_livoxCtrl.m_vRz.y + 0.1, m_livoxCtrl.m_vRz.x, 10);
                                            m_cbCamCtrl.call(&m_livoxCtrl);
                                            UpdateBtnState(); });
                Button *pBtnMaxDdec = new Button(" - ");
                pBtnMaxDdec->SetOnClicked([this]()
                                          {
                                            m_livoxCtrl.m_vRz.y = constrain<float>(m_livoxCtrl.m_vRz.y - 0.1, m_livoxCtrl.m_vRz.x, 10);
                                            m_cbCamCtrl.call(&m_livoxCtrl);
                                            UpdateBtnState(); });
                m_pBtnMaxD->SetPaddingEm(btnPadH, btnPadV);
                pBtnMaxDinc->SetPaddingEm(btnPadH, btnPadV);
                pBtnMaxDdec->SetPaddingEm(btnPadH, btnPadV);
                pGc->AddChild(GiveOwnership(m_pBtnMaxD));
                pGc->AddChild(GiveOwnership(pBtnMaxDinc));
                pGc->AddChild(GiveOwnership(pBtnMaxDdec));

                panelFilter->AddChild(GiveOwnership(pGc));

            }

            void _LivoxScanAutoUI::OnSavePLY(void)
            {
                // string fName = getBaseDirSave();
                // if (fName.empty())
                // {
                //     ShowMsg("FILE", "Insert USB memory", true);
                //     return;
                // }

                // fName += tFormat();
                // string imgName = fName + ".png";
                // this->ExportCurrentImage(imgName.c_str());
                // m_cbSavePC.call(&fName);

                m_cbSavePC.call();

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

        } // namespace visualizer
    }     // namespace visualization
} // namespace open3d
