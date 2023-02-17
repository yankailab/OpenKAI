#ifdef USE_OPEN3D
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
                //                Init();
            }

            _VzScanUI::~_VzScanUI() {}

            void _VzScanUI::Init(void)
            {
                this->O3DUI::Init();

                m_bScanning = true;
                m_bCamAuto = false;
                m_areaName = "PCAREA";

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
//                m_btnScan->SetOn(m_bScanning);
                m_btnScanReset->SetOn(m_bScanning);
                m_btnScanTake->SetOn(m_bScanning);
                m_btnCamAuto->SetOn(m_bCamAuto);
                m_btnCamAuto->SetEnabled(m_bScanning);
                m_btnCamAll->SetEnabled(!m_bCamAuto);
                m_btnCamOrigin->SetEnabled(!m_bCamAuto);
                m_btnCamL->SetEnabled(!m_bCamAuto);
                m_btnCamR->SetEnabled(!m_bCamAuto);
                m_btnCamF->SetEnabled(!m_bCamAuto);
                m_btnCamB->SetEnabled(!m_bCamAuto);
                m_btnCamU->SetEnabled(!m_bCamAuto);
                m_btnCamD->SetEnabled(!m_bCamAuto);

                m_btnOpenPC->SetEnabled(!m_bScanning);
                m_btnSavePC->SetEnabled(!m_bScanning);
                m_btnSaveRGB->SetEnabled(!m_bScanning);
                m_sliderVsize->SetEnabled(!m_bScanning);
                m_btnHiddenRemove->SetEnabled(!m_bScanning);
                m_btnFilterReset->SetEnabled(!m_bScanning);

                // if (m_bScanning)
                // {
                //     m_btnScan->SetText("        Stop        ");
                //     SetMouseCameraMode();
                // }
                // else
                // {
                //     m_btnScan->SetText("        Start        ");
                // }

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

            // void _VzScanUI::SetCbScan(OnCbO3DUI pCb, void *pPCV)
            // {
            //     m_cbScan.add(pCb, pPCV);
            // }

            void _VzScanUI::SetCbScanReset(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbScanReset.add(pCb, pPCV);
            }

            void _VzScanUI::SetCbScanTake(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbScanTake.add(pCb, pPCV);
            }

            void _VzScanUI::SetCbOpenPC(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbOpenPC.add(pCb, pPCV);
            }

            void _VzScanUI::SetCbCamSet(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbCamSet.add(pCb, pPCV);
            }

            void _VzScanUI::SetCbVoxelDown(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbVoxelDown.add(pCb, pPCV);
            }

            void _VzScanUI::SetCbHiddenRemove(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbHiddenRemove.add(pCb, pPCV);
            }

            void _VzScanUI::SetCbResetPC(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbResetPC.add(pCb, pPCV);
            }

            void _VzScanUI::Layout(const gui::LayoutContext &context)
            {
                int settings_width = m_uiState.m_wPanel * context.theme.font_size;

                auto f = GetContentRect();
                if (m_panelCtrl->IsVisible())
                {
                    m_pScene->SetFrame(Rect(f.x, f.y, f.width - settings_width, f.height));
                    m_panelCtrl->SetFrame(Rect(f.GetRight() - settings_width, f.y, settings_width, f.height));
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

                // File
                auto panelFile = new CollapsableVert("FILE", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelFile));

                m_btnOpenPC = new Button(" Open ");
                m_btnOpenPC->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnOpenPC->SetOnClicked([this]()
                                          { OnOpenPLY(); });

                m_btnSavePC = new Button(" Save ");
                m_btnSavePC->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnSavePC->SetOnClicked([this]()
                                          { OnSavePLY(); });

                m_btnSaveRGB = new Button("Screen");
                m_btnSaveRGB->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnSaveRGB->SetOnClicked([this]()
                                           { OnSaveRGB(); });

                auto pH = new Horiz(v_spacing);
                pH->AddChild(GiveOwnership(m_btnOpenPC));
                pH->AddChild(GiveOwnership(m_btnSavePC));
                pH->AddChild(GiveOwnership(m_btnSaveRGB));
                pH->AddStretch();
                panelFile->AddChild(GiveOwnership(pH));

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
                                               m_pScene->ForceRedraw();
                                           });

                m_btnCamAll = new Button("    All    ");
                m_btnCamAll->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnCamAll->SetOnClicked([this]()
                                          {
                                              int m = 4;
                                              m_cbCamSet.call(&m);
                                              if (m_uiMode == uiMode_pointPick)
                                                  UpdateSelectableGeometry();
                                              m_pScene->ForceRedraw();
                                          });

                m_btnCamOrigin = new Button(" Origin ");
                m_btnCamOrigin->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnCamOrigin->SetOnClicked([this]()
                                             {
                                                 int m = 3;
                                                 m_cbCamSet.call(&m);
                                                 if (m_uiMode == uiMode_pointPick)
                                                     UpdateSelectableGeometry();
                                                 m_pScene->ForceRedraw();
                                             });

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

                // View
                auto panelView = new CollapsableVert("VIEW", v_spacing, margins);
                panelView->SetIsOpen(true);
                m_panelCtrl->AddChild(GiveOwnership(panelView));

                auto sliderPointSize = new Slider(Slider::INT);
                sliderPointSize->SetLimits(1, 10);
                sliderPointSize->SetValue(m_uiState.m_sPoint);
                sliderPointSize->SetOnValueChanged([this](const double v)
                                                   { SetPointSize(int(v)); });

                m_sliderVsize = new Slider(Slider::DOUBLE);
                m_sliderVsize->SetLimits(0.0, 1.0);
                m_sliderVsize->SetValue(m_uiState.m_sVoxel);
                m_sliderVsize->SetOnValueChanged([this](const double v)
                                                 {
                                                     m_uiState.m_sVoxel = v;
                                                     m_cbVoxelDown.call(&m_uiState);
                                                     m_pScene->ForceRedraw();
                                                 });

                auto *pG = new VGrid(2, v_spacing);
                pG->AddChild(make_shared<Label>("PointSize"));
                pG->AddChild(GiveOwnership(sliderPointSize));
                pG->AddChild(make_shared<Label>("VoxelSize"));
                pG->AddChild(GiveOwnership(m_sliderVsize));
                panelView->AddChild(GiveOwnership(pG));

                m_btnHiddenRemove = new Button(" Z-Cull ");
                m_btnHiddenRemove->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnHiddenRemove->SetOnClicked([this]()
                                                {
                                                    m_uiState.m_vCamPos = m_pScene->GetScene()->GetCamera()->GetPosition();
                                                    m_cbHiddenRemove.call(&m_uiState);
                                                    m_pScene->ForceRedraw();
                                                });

                m_btnFilterReset = new Button(" Reset ");
                m_btnFilterReset->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnFilterReset->SetOnClicked([this]()
                                               {
                                                   m_cbResetPC.call();
                                                   m_pScene->ForceRedraw();
                                               });

                pG = new VGrid(2, v_spacing);
                pG->AddChild(GiveOwnership(m_btnHiddenRemove));
                pG->AddChild(GiveOwnership(m_btnFilterReset));
                panelView->AddChild(GiveOwnership(pG));

                // Scan
                auto panelScan = new CollapsableVert("SCAN", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelScan));

                // m_btnScan = new Button("        Start        ");
                // m_btnScan->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                // m_btnScan->SetToggleable(true);
                // m_btnScan->SetOnClicked([this]()
                //                         {
                //                             m_bScanning = !m_bScanning;
                //                             m_cbScan.call(&m_bScanning);
                //                             m_bCamAuto = m_bScanning;
                //                             int m = m_bCamAuto ? 1 : 0;
                //                             m_cbCamSet.call(&m);
                //                             UpdateBtnState();
                //                             PostRedraw();
                //                         });
                // pH = new Horiz(v_spacing);
                // pH->AddChild(GiveOwnership(m_btnScan));
                // pH->AddStretch();
                // panelScan->AddChild(GiveOwnership(pH));


                m_btnScanReset = new Button("        Reset        ");
                m_btnScanReset->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnScanReset->SetOnClicked([this]()
                                        {
                                            m_cbScanReset.call();
                                            m_bCamAuto = m_bScanning;
                                            int m = m_bCamAuto ? 1 : 0;
                                            m_cbCamSet.call(&m);
                                            UpdateBtnState();
                                            PostRedraw();
                                        });

                m_btnScanTake = new Button("        Take        ");
                m_btnScanTake->SetPaddingEm(m_uiState.m_btnPaddingH, m_uiState.m_btnPaddingV);
                m_btnScanTake->SetOnClicked([this]()
                                        {
                                            m_cbScanTake.call();
//                                            PostRedraw();
//                                            m_pScene->ForceRedraw();
                                        });

                pG = new VGrid(2, v_spacing);
                pG->AddChild(GiveOwnership(m_btnScanReset));
                pG->AddChild(GiveOwnership(m_btnScanTake));
                panelScan->AddChild(GiveOwnership(pG));

                m_labelProg = new Label("Memory used: 0%");
                pH = new Horiz(v_spacing);
                pH->AddChild(GiveOwnership(m_labelProg));
                pH->AddStretch();
                panelScan->AddChild(GiveOwnership(pH));

                m_progScan = new ProgressBar();
                m_progScan->SetValue(0.0);
                pH = new Horiz(v_spacing);
                pH->AddChild(GiveOwnership(m_progScan));
                panelScan->AddChild(GiveOwnership(pH));
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
                    ShowMsg("File Save", "Insert USB memory", true);
                    return;
                }

                fName += tFormat();
                string imgName = fName + ".png";
                string plyName = fName + ".ply";

                this->ExportCurrentImage(imgName.c_str());

                io::WritePointCloudOption par;
                par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
                par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

                shared_ptr<t::geometry::PointCloud> spTpc = GetGeometry(m_modelName).m_sTgeometry;
                io::WritePointCloudToPLY(plyName.c_str(), spTpc->ToLegacy(), par);

                ShowMsg("File Save", "Saved model to USB memory", true);

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
                                   this->m_cbOpenPC.call((void *)path);
                               });
                ShowDialog(dlg);
            }

        } // namespace visualizer
    }     // namespace visualization
} // namespace open3d

#endif