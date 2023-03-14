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
                m_btnScanReset->SetEnabled(true);
                m_btnScanTake->SetEnabled(true);
                m_btnSavePC->SetEnabled(true);

                m_btnCamAuto->SetOn(m_bCamAuto);
                m_btnCamAuto->SetEnabled(true);
                m_btnCamAll->SetEnabled(!m_bCamAuto);
                m_btnCamOrigin->SetEnabled(!m_bCamAuto);
                m_btnCamL->SetEnabled(!m_bCamAuto);
                m_btnCamR->SetEnabled(!m_bCamAuto);
                m_btnCamF->SetEnabled(!m_bCamAuto);
                m_btnCamB->SetEnabled(!m_bCamAuto);
                m_btnCamU->SetEnabled(!m_bCamAuto);
                m_btnCamD->SetEnabled(!m_bCamAuto);

                m_btnFillHole->SetEnabled(true);
                m_btnFilSpatial->SetEnabled(true);
                m_btnHDR->SetEnabled(true);
                m_btnFillHole->SetText(m_camCtrl.m_bFillHole ? "Fill hole ON" : "Fill hole OFF");
                m_btnFilSpatial->SetText(m_camCtrl.m_bSpatialFilter ? "Spatial ON" : "Spatial OFF");
                m_btnHDR->SetText(m_camCtrl.m_bHDR ? "HDR ON" : "HDR OFF");

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

                m_btnCamAuto = new Button("  Auto  ");
                m_btnCamAuto->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnCamAuto->SetOnClicked([this]()
                                           {
                                               m_bCamAuto = !m_bCamAuto;
                                               int m = m_bCamAuto ? 1 : 0;
                                               m_cbCamSet.call(&m);
                                               UpdateBtnState();
                                               m_pScene->ForceRedraw(); });

                m_btnCamAll = new Button("    All    ");
                m_btnCamAll->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnCamAll->SetOnClicked([this]()
                                          {
                                              int m = 4;
                                              m_cbCamSet.call(&m);
                                              if (m_uiMode == uiMode_pointPick)
                                                  UpdateSelectableGeometry();
                                              m_pScene->ForceRedraw(); });

                m_btnCamOrigin = new Button(" Origin ");
                m_btnCamOrigin->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnCamOrigin->SetOnClicked([this]()
                                             {
                                                 int m = 3;
                                                 m_cbCamSet.call(&m);
                                                 if (m_uiMode == uiMode_pointPick)
                                                     UpdateSelectableGeometry();
                                                 m_pScene->ForceRedraw(); });

                m_btnCamL = new Button(" < ");
                m_btnCamL->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnCamL->SetOnClicked([this]()
                                        { camMove(Vector3f(-1, 0, 0)); });

                m_btnCamR = new Button(" > ");
                m_btnCamR->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnCamR->SetOnClicked([this]()
                                        { camMove(Vector3f(1, 0, 0)); });

                m_btnCamF = new Button(" ^ ");
                m_btnCamF->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnCamF->SetOnClicked([this]()
                                        { camMove(Vector3f(0, 0, -1)); });

                m_btnCamB = new Button(" v ");
                m_btnCamB->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnCamB->SetOnClicked([this]()
                                        { camMove(Vector3f(0, 0, 1)); });

                m_btnCamU = new Button(" Up ");
                m_btnCamU->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnCamU->SetOnClicked([this]()
                                        { camMove(Vector3f(0, 1, 0)); });

                m_btnCamD = new Button(" Down ");
                m_btnCamD->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnCamD->SetOnClicked([this]()
                                        { camMove(Vector3f(0, -1, 0)); });

                auto *pN = new VGrid(3, v_spacing);
                pN->AddChild(GiveOwnership(m_btnCamAuto));
                pN->AddChild(GiveOwnership(m_btnCamAll));
                pN->AddChild(GiveOwnership(m_btnCamOrigin));
                pN->AddChild(GiveOwnership(m_btnCamD));
                pN->AddChild(GiveOwnership(m_btnCamF));
                pN->AddChild(GiveOwnership(m_btnCamU));
                pN->AddChild(GiveOwnership(m_btnCamL));
                pN->AddChild(GiveOwnership(m_btnCamB));
                pN->AddChild(GiveOwnership(m_btnCamR));
                panelCam->AddChild(GiveOwnership(pN));

                // Scan
                auto panelScan = new CollapsableVert("SCAN", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelScan));

                m_btnScanReset = new Button(" Reset ");
                m_btnScanReset->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnScanReset->SetOnClicked([this]()
                                             {
                                            m_cbScanReset.call();
                                            int m = m_bCamAuto ? 1 : 0;
                                            m_cbCamSet.call(&m);
                                            UpdateBtnState();
                                            PostRedraw(); });

                m_btnSavePC = new Button("  Save  ");
                m_btnSavePC->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnSavePC->SetOnClicked([this]()
                                          { OnSavePLY(); });

                m_btnScanTake = new Button("  Take  ");
                m_btnScanTake->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnScanTake->SetOnClicked([this]()
                                            { m_cbScanTake.call(); });

                auto *pG = new VGrid(3, v_spacing);
                pG->AddChild(GiveOwnership(m_btnScanReset));
                pG->AddChild(GiveOwnership(m_btnSavePC));
                pG->AddChild(GiveOwnership(m_btnScanTake));
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

                // Control
                auto panelCtrl = new CollapsableVert("CONTROL", v_spacing, margins);
                panelCtrl->SetIsOpen(true);
                m_panelCtrl->AddChild(GiveOwnership(panelCtrl));

                pG = new VGrid(3, v_spacing);
                Button *pBtn;

                m_pLbPointSize = new Label("Point Size: ");
                pG->AddChild(GiveOwnership(m_pLbPointSize));
                pBtn = new Button("+");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_pointSize = constrain(m_pointSize+1, 1, 10);
                                        SetPointSize(m_pointSize);
                                        m_pLbPointSize->SetText(string("Point Size: " + i2str(m_pointSize)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));
                pBtn = new Button("-");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_pointSize = constrain(m_pointSize-1, 1, 10);
                                        SetPointSize(m_pointSize);
                                        m_pLbPointSize->SetText(string("Point Size: " + i2str(m_pointSize)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));

                m_pLbMinD = new Label("Min distance: ");
                pG->AddChild(GiveOwnership(m_pLbMinD));
                pBtn = new Button("+");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_vRz.x = constrain<float>(m_camCtrl.m_vRz.x + 0.1, 1, m_camCtrl.m_vRz.y);
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbMinD->SetText(string("Min distance: " + f2str(m_camCtrl.m_vRz.x)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));
                pBtn = new Button("-");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_vRz.x = constrain<float>(m_camCtrl.m_vRz.x - 0.1, 1, 10);
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbMinD->SetText(string("Min distance: " + f2str(m_camCtrl.m_vRz.x)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));

                m_pLbMaxD = new Label("Max distance: ");
                pG->AddChild(GiveOwnership(m_pLbMaxD));
                pBtn = new Button("+");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_vRz.y = constrain<float>(m_camCtrl.m_vRz.y + 0.1, 0.0, 10);
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbMaxD->SetText(string("Min distance: " + f2str(m_camCtrl.m_vRz.y)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));
                pBtn = new Button("-");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_vRz.y = constrain<float>(m_camCtrl.m_vRz.y - 0.1, m_camCtrl.m_vRz.x, 10);
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbMaxD->SetText(string("Min distance: " + f2str(m_camCtrl.m_vRz.y)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));

                m_pLbToFexp = new Label("ToF exposure: ");
                pG->AddChild(GiveOwnership(m_pLbToFexp));
                pBtn = new Button("+");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_tExposureToF = constrain<int>(m_camCtrl.m_tExposureToF += 1000, 0, 4000);
                                        m_camCtrl.m_bAutoExposureToF = (m_camCtrl.m_tExposureToF>0)?false:true;
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbToFexp->SetText(string("ToF exposure: " + i2str(m_camCtrl.m_tExposureToF)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));
                pBtn = new Button("-");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_tExposureToF = constrain<int>(m_camCtrl.m_tExposureToF -= 1000, 0, 4000);
                                        m_camCtrl.m_bAutoExposureToF = (m_camCtrl.m_tExposureToF>0)?false:true;
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbToFexp->SetText(string("ToF exposure: " + i2str(m_camCtrl.m_tExposureToF)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));


                m_pLbRGBexp = new Label("RGB exposure: ");
                pG->AddChild(GiveOwnership(m_pLbRGBexp));
                pBtn = new Button("+");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_tExposureRGB = constrain<int>(m_camCtrl.m_tExposureRGB += 1000, 0, 4000);
                                        m_camCtrl.m_bAutoExposureRGB = (m_camCtrl.m_tExposureRGB>0)?false:true;
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbRGBexp->SetText(string("RGB exposure: " + i2str(m_camCtrl.m_tExposureRGB)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));
                pBtn = new Button("-");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_tExposureRGB = constrain<int>(m_camCtrl.m_tExposureRGB -= 1000, 0, 4000);
                                        m_camCtrl.m_bAutoExposureRGB = (m_camCtrl.m_tExposureRGB>0)?false:true;
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbRGBexp->SetText(string("RGB exposure: " + i2str(m_camCtrl.m_tExposureRGB)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));


                m_pLbTfilter = new Label("Time filter: ");
                pG->AddChild(GiveOwnership(m_pLbTfilter));
                pBtn = new Button("+");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_filTime = constrain<int>(m_camCtrl.m_filTime += 1, 0, 3);
                                        m_camCtrl.m_bFilTime = (m_camCtrl.m_filTime>0)?false:true;
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbTfilter->SetText(string("Time filter: " + i2str(m_camCtrl.m_filTime)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));
                pBtn = new Button("-");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_filTime = constrain<int>(m_camCtrl.m_filTime -= 1, 0, 3);
                                        m_camCtrl.m_bFilTime = (m_camCtrl.m_filTime>0)?false:true;
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbTfilter->SetText(string("Time filter: " + i2str(m_camCtrl.m_filTime)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));


                m_pLbCfilter = new Label("Confidence filter: ");
                pG->AddChild(GiveOwnership(m_pLbCfilter));
                pBtn = new Button("+");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_filConfidence = constrain<int>(m_camCtrl.m_filConfidence += 1, 0, 100);
                                        m_camCtrl.m_bFilConfidence = (m_camCtrl.m_filConfidence>0)?false:true;
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbCfilter->SetText(string("Confidence filter: " + i2str(m_camCtrl.m_filConfidence)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));
                pBtn = new Button("-");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_filConfidence = constrain<int>(m_camCtrl.m_filConfidence -= 1, 0, 100);
                                        m_camCtrl.m_bFilConfidence = (m_camCtrl.m_filConfidence>0)?false:true;
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbCfilter->SetText(string("Confidence filter: " + i2str(m_camCtrl.m_filConfidence)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));


                m_pLbFpFilter = new Label("Flying pixel filter: ");
                pG->AddChild(GiveOwnership(m_pLbFpFilter));
                pBtn = new Button("+");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_filFlyingPix = constrain<int>(m_camCtrl.m_filFlyingPix += 1, 0, 49);
                                        m_camCtrl.m_bFilFlyingPix = (m_camCtrl.m_filFlyingPix>0)?false:true;
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbFpFilter->SetText(string("Flying pixel filter: " + i2str(m_camCtrl.m_filFlyingPix)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));
                pBtn = new Button("-");
                pBtn->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                pBtn->SetOnClicked([this]()
                                   {
                                        m_camCtrl.m_filFlyingPix = constrain<int>(m_camCtrl.m_filFlyingPix -= 1, 0, 49);
                                        m_camCtrl.m_bFilFlyingPix = (m_camCtrl.m_filFlyingPix>0)?false:true;
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        m_pLbFpFilter->SetText(string("Flying pixel filter: " + i2str(m_camCtrl.m_filFlyingPix)).c_str());
                                        m_pScene->ForceRedraw(); });
                pG->AddChild(GiveOwnership(pBtn));


                panelCtrl->AddChild(GiveOwnership(pG));

                m_btnFillHole = new Button("Fill hole");
                m_btnFillHole->SetPaddingEm(m_uiState.m_btnPaddingH / 2, m_uiState.m_btnPaddingV);
                m_btnFillHole->SetOnClicked([this]()
                                            {
                                        m_camCtrl.m_bFillHole = !m_camCtrl.m_bFillHole;
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        UpdateBtnState();
                                        m_pScene->ForceRedraw(); });

                m_btnFilSpatial = new Button("Spatial filter");
                m_btnFilSpatial->SetPaddingEm(m_uiState.m_btnPaddingH / 2, m_uiState.m_btnPaddingV);
                m_btnFilSpatial->SetOnClicked([this]()
                                              {
                                        m_camCtrl.m_bSpatialFilter = !m_camCtrl.m_bSpatialFilter;
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        UpdateBtnState();
                                        m_pScene->ForceRedraw(); });

                m_btnHDR = new Button(" HDR ");
                m_btnHDR->SetPaddingEm(m_uiState.m_btnPaddingH / 2, m_uiState.m_btnPaddingV);
                m_btnHDR->SetOnClicked([this]()
                                       {
                                        m_camCtrl.m_bHDR = !m_camCtrl.m_bHDR;
                                        m_cbCamCtrl.call(&m_camCtrl);
                                        UpdateBtnState();
                                        m_pScene->ForceRedraw(); });

                pG = new VGrid(3, v_spacing);
                pG->AddChild(GiveOwnership(m_btnFillHole));
                pG->AddChild(GiveOwnership(m_btnFilSpatial));
                pG->AddChild(GiveOwnership(m_btnHDR));
                panelCtrl->AddChild(GiveOwnership(pG));
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
