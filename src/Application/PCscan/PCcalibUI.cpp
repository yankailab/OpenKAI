#ifdef USE_OPEN3D
#include "PCcalibUI.h"

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

            PCcalibUI::PCcalibUI(const string &title, int width, int height)
                : O3DUI(title, width, height)
            {
                Init();
            }

            PCcalibUI::~PCcalibUI() {}

            void PCcalibUI::Init(void)
            {
                this->O3DUI::Init();

                m_bScanning = false;
                m_dV = 0.01;
                m_nD = 15;
                InitCtrlPanel();
                UpdateUIstate();

                m_pScene->SetViewControls(m_uiState.m_mouseMode);

                auto menu = std::make_shared<Menu>();
                auto file_menu = std::make_shared<Menu>();
                file_menu->AddItem("Import params", MENU_IMPORT_PARAMS);
                file_menu->AddItem("Export params", MENU_EXPORT_PARAMS);
                menu->AddMenu("File", file_menu);

                Application::GetInstance().SetMenubar(menu);
                SetOnMenuItemActivated(MENU_IMPORT_PARAMS,
                                       [this]() { OnImportParams(); });
                SetOnMenuItemActivated(MENU_EXPORT_PARAMS,
                                       [this]() { OnExportParams(); });
            }

            void PCcalibUI::UpdateUIstate(void)
            {
                m_panelCtrl->SetVisible(m_uiState.m_bShowPanel);
                this->O3DUI::UpdateUIstate();
            }

            PCCALIB_PARAM *PCcalibUI::GetCalibParams(void)
            {
                return &m_cParam;
            }

            void PCcalibUI::UpdateCalibParams(void)
            {
                m_pFx->SetText(lf2str(m_cParam.m_Fx,m_nD).c_str());
                m_pFy->SetText(lf2str(m_cParam.m_Fy,m_nD).c_str());
                m_pCx->SetText(lf2str(m_cParam.m_Cx,m_nD).c_str());
                m_pCy->SetText(lf2str(m_cParam.m_Cy,m_nD).c_str());

                m_pk1->SetText(lf2str(m_cParam.m_k1,m_nD).c_str());
                m_pk2->SetText(lf2str(m_cParam.m_k2,m_nD).c_str());
                m_pp1->SetText(lf2str(m_cParam.m_p1,m_nD).c_str());
                m_pp2->SetText(lf2str(m_cParam.m_p2,m_nD).c_str());
                m_pk3->SetText(lf2str(m_cParam.m_k3,m_nD).c_str());

                m_pCoTx->SetText(lf2str(m_cParam.m_coTx,m_nD).c_str());
                m_pCoTy->SetText(lf2str(m_cParam.m_coTy,m_nD).c_str());
                m_pCoTz->SetText(lf2str(m_cParam.m_coTz,m_nD).c_str());
                m_pCoRx->SetText(lf2str(m_cParam.m_coRx,m_nD).c_str());
                m_pCoRy->SetText(lf2str(m_cParam.m_coRy,m_nD).c_str());
                m_pCoRz->SetText(lf2str(m_cParam.m_coRz,m_nD).c_str());

                m_pPoTx->SetText(lf2str(m_cParam.m_poTx,m_nD).c_str());
                m_pPoTy->SetText(lf2str(m_cParam.m_poTy,m_nD).c_str());
                m_pPoTz->SetText(lf2str(m_cParam.m_poTz,m_nD).c_str());
                m_pPoRx->SetText(lf2str(m_cParam.m_poRx,m_nD).c_str());
                m_pPoRy->SetText(lf2str(m_cParam.m_poRy,m_nD).c_str());
                m_pPoRz->SetText(lf2str(m_cParam.m_poRz,m_nD).c_str());
            }

            void PCcalibUI::SetCbScan(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbScan.add(pCb, pPCV);
            }

            void PCcalibUI::SetCbResetPC(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbResetPC.add(pCb, pPCV);
            }

            void PCcalibUI::SetCbRefreshCol(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbRefreshCol.add(pCb, pPCV);
            }

            void PCcalibUI::SetCbLoadImgs(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbLoadImgs.add(pCb, pPCV);
            }

            void PCcalibUI::SetCbUpdateParams(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbUpdateParams.add(pCb, pPCV);
            }

            void PCcalibUI::SetCbImportParams(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbImportParams.add(pCb, pPCV);
            }

            void PCcalibUI::SetCbExportParams(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbExportParams.add(pCb, pPCV);
            }

            void PCcalibUI::Layout(const gui::LayoutContext &context)
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

            void PCcalibUI::InitCtrlPanel(void)
            {
                auto em = GetTheme().font_size;
                auto half_em = int(round(0.5f * float(em)));
                auto v_spacing = int(round(0.25 * float(em)));

                m_panelCtrl = new Vert(half_em);
                AddChild(GiveOwnership(m_panelCtrl));
                Margins margins(em, 0, half_em, 0);

                // Point cloud
                auto panelPC = new CollapsableVert("Point Cloud", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelPC));

                auto pH = new Horiz(v_spacing);
                panelPC->AddChild(GiveOwnership(pH));

                m_btnScan = new Button(" Start ");
                m_btnScan->SetToggleable(true);
                m_btnScan->SetOnClicked([this]()
                {
                    m_bScanning = !m_bScanning;
                    m_btnScan->SetText((m_bScanning)?" Stop ":" Start ");
                    m_cbScan.call(&m_bScanning);
                    PostRedraw();
                });
                pH->AddChild(GiveOwnership(m_btnScan));

                auto btnReset = new Button(" Reset ");
                btnReset->SetOnClicked([this]() {
                    m_cbResetPC.call();
                });
                pH->AddChild(GiveOwnership(btnReset));

                auto btnRefreshCol = new Button(" Refresh Color ");
                btnRefreshCol->SetOnClicked([this]() {
                    m_cbRefreshCol.call();
                });
                pH->AddChild(GiveOwnership(btnRefreshCol));

                pH->AddStretch();

                auto sliderPointSize = new Slider(Slider::INT);
                sliderPointSize->SetLimits(1, 10);
                sliderPointSize->SetValue(m_uiState.m_sPoint);
                sliderPointSize->SetOnValueChanged([this](const double v) {
                    SetPointSize(int(v));
                });
                auto *pG = new VGrid(2, v_spacing);
                pG->AddChild(make_shared<Label>("PointSize"));
                pG->AddChild(GiveOwnership(sliderPointSize));
                panelPC->AddChild(GiveOwnership(pG));

                // Calibration using images
                auto panelImg = new CollapsableVert("Calibration", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelImg));

                pH = new Horiz(v_spacing);
                panelImg->AddChild(GiveOwnership(pH));
                m_pPath = new TextEdit();
                m_pPath->SetText("/home/");
                pH->AddChild(GiveOwnership(m_pPath));
                pH->AddStretch();

                pH = new Horiz(v_spacing);
                panelImg->AddChild(GiveOwnership(pH));
                auto btnCalib = new Button(" Calibrate ");
                btnCalib->SetOnClicked([this]() { 
                    ShowMsg("Camera calibration", "Calibrating");
                    m_cbLoadImgs.call((void *)m_pPath->GetText());
                    CloseMsg();
                });
                pH->AddChild(GiveOwnership(btnCalib));
                pH->AddStretch();

                // Cam Intrinsic
                auto panelCam = new CollapsableVert("Cam Intrinsic", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelCam));

                pG = new VGrid(2, v_spacing);
                panelCam->AddChild(GiveOwnership(pG));

                m_pFx = new TextEdit();
                m_pFx->SetText("0.0");
                m_pFx->SetOnTextChanged([this](const char *) {
                    m_cParam.m_Fx = atof(m_pFx->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("Fx")));
                pG->AddChild(GiveOwnership(m_pFx));

                m_pFy = new TextEdit();
                m_pFy->SetText("0.0");
                m_pFy->SetOnTextChanged([this](const char *) {
                    m_cParam.m_Fy = atof(m_pFy->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("Fy")));
                pG->AddChild(GiveOwnership(m_pFy));

                m_pCx = new TextEdit();
                m_pCx->SetText("0.0");
                m_pCx->SetOnTextChanged([this](const char *) {
                    m_cParam.m_Cx = atof(m_pCx->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("Cx")));
                pG->AddChild(GiveOwnership(m_pCx));

                m_pCy = new TextEdit();
                m_pCy->SetText("0.0");
                m_pCy->SetOnTextChanged([this](const char *) {
                    m_cParam.m_Cy = atof(m_pCy->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("Cy")));
                pG->AddChild(GiveOwnership(m_pCy));

                // Cam distortion
                auto panelDistort = new CollapsableVert("Cam Distortion", v_spacing, margins);
                panelDistort->SetIsOpen(false);
                m_panelCtrl->AddChild(GiveOwnership(panelDistort));
                pG = new VGrid(2, v_spacing);
                panelDistort->AddChild(GiveOwnership(pG));

                m_pk1 = new TextEdit();
                m_pk1->SetText("0.0");
                m_pk1->SetOnTextChanged([this](const char *) {
                    m_cParam.m_k1 = atof(m_pk1->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("k1")));
                pG->AddChild(GiveOwnership(m_pk1));

                m_pk2 = new TextEdit();
                m_pk2->SetText("0.0");
                m_pk2->SetOnTextChanged([this](const char *) {
                    m_cParam.m_k2 = atof(m_pk2->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("k2")));
                pG->AddChild(GiveOwnership(m_pk2));

                m_pp1 = new TextEdit();
                m_pp1->SetText("0.0");
                m_pp1->SetOnTextChanged([this](const char *) {
                    m_cParam.m_p1 = atof(m_pp1->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("p1")));
                pG->AddChild(GiveOwnership(m_pp1));

                m_pp2 = new TextEdit();
                m_pp2->SetText("0.0");
                m_pp2->SetOnTextChanged([this](const char *) {
                    m_cParam.m_p2 = atof(m_pp2->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("p2")));
                pG->AddChild(GiveOwnership(m_pp2));

                m_pk3 = new TextEdit();
                m_pk3->SetText("0.0");
                m_pk3->SetOnTextChanged([this](const char *) {
                    m_cParam.m_k3 = atof(m_pk3->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("k3")));
                pG->AddChild(GiveOwnership(m_pk3));

                // RGB offset
                auto panelOffsetC = new CollapsableVert("RGB offset", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelOffsetC));

                pG = new VGrid(2, v_spacing);
                panelOffsetC->AddChild(GiveOwnership(pG));

                m_pCoTx = new TextEdit();
                m_pCoTx->SetText("0.0");
                m_pCoTx->SetOnTextChanged([this](const char *) {
                    m_cParam.m_coTx = atof(m_pCoTx->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("tX")));
                pG->AddChild(GiveOwnership(m_pCoTx));

                m_pCoTy = new TextEdit();
                m_pCoTy->SetText("0.0");
                m_pCoTy->SetOnTextChanged([this](const char *) {
                    m_cParam.m_coTy = atof(m_pCoTy->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("tY")));
                pG->AddChild(GiveOwnership(m_pCoTy));

                m_pCoTz = new TextEdit();
                m_pCoTz->SetText("0.0");
                m_pCoTz->SetOnTextChanged([this](const char *) {
                    m_cParam.m_coTz = atof(m_pCoTz->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("tZ")));
                pG->AddChild(GiveOwnership(m_pCoTz));

                m_pCoRx = new TextEdit();
                m_pCoRx->SetText("0.0");
                m_pCoRx->SetOnTextChanged([this](const char *) {
                    m_cParam.m_coRx = atof(m_pCoRx->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("rX")));
                pG->AddChild(GiveOwnership(m_pCoRx));

                m_pCoRy = new TextEdit();
                m_pCoRy->SetText("0.0");
                m_pCoRy->SetOnTextChanged([this](const char *) {
                    m_cParam.m_coRy = atof(m_pCoRy->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("rY")));
                pG->AddChild(GiveOwnership(m_pCoRy));

                m_pCoRz = new TextEdit();
                m_pCoRz->SetText("0.0");
                m_pCoRz->SetOnTextChanged([this](const char *) {
                    m_cParam.m_coRz = atof(m_pCoRz->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("rZ")));
                pG->AddChild(GiveOwnership(m_pCoRz));

                // Slam offset
                auto panelOffsetP = new CollapsableVert("SLAM offset", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelOffsetP));

                pG = new VGrid(2, v_spacing);
                panelOffsetP->AddChild(GiveOwnership(pG));

                m_pPoTx = new TextEdit();
                m_pPoTx->SetText("0.0");
                m_pPoTx->SetOnTextChanged([this](const char *) {
                    m_cParam.m_poTx = atof(m_pPoTx->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("tX")));
                pG->AddChild(GiveOwnership(m_pPoTx));

                m_pPoTy = new TextEdit();
                m_pPoTy->SetText("0.0");
                m_pPoTy->SetOnTextChanged([this](const char *) {
                    m_cParam.m_poTy = atof(m_pPoTy->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("tY")));
                pG->AddChild(GiveOwnership(m_pPoTy));

                m_pPoTz = new TextEdit();
                m_pPoTz->SetText("0.0");
                m_pPoTz->SetOnTextChanged([this](const char *) {
                    m_cParam.m_poTz = atof(m_pPoTz->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("tZ")));
                pG->AddChild(GiveOwnership(m_pPoTz));

                m_pPoRx = new TextEdit();
                m_pPoRx->SetText("0.0");
                m_pPoRx->SetOnTextChanged([this](const char *) {
                    m_cParam.m_poRx = atof(m_pPoRx->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("rX")));
                pG->AddChild(GiveOwnership(m_pPoRx));

                m_pPoRy = new TextEdit();
                m_pPoRy->SetText("0.0");
                m_pPoRy->SetOnTextChanged([this](const char *) {
                    m_cParam.m_poRy = atof(m_pPoRy->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("rY")));
                pG->AddChild(GiveOwnership(m_pPoRy));

                m_pPoRz = new TextEdit();
                m_pPoRz->SetText("0.0");
                m_pPoRz->SetOnTextChanged([this](const char *) {
                    m_cParam.m_poRz = atof(m_pPoRz->GetText());
                    m_cbUpdateParams.call();
                });
                pG->AddChild(GiveOwnership(new Label("rZ")));
                pG->AddChild(GiveOwnership(m_pPoRz));
            }

            void PCcalibUI::OnImportParams(void)
            {
                auto dlg = make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::OPEN, "Open Calibration File", this->GetTheme());
                dlg->AddFilter(".kiss", "Calibration file (.kiss)");
                dlg->AddFilter("", "All files");
                dlg->SetOnCancel([this]() { this->CloseDialog(); });
                dlg->SetOnDone([this](const char *path) {
                    this->m_cbImportParams.call((void*)path);
                    this->CloseDialog();
                });
                ShowDialog(dlg);
            }

            void PCcalibUI::OnExportParams(void)
            {
                auto dlg = make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::SAVE, "Save File", this->GetTheme());
                dlg->AddFilter(".kiss", "Calibration file (.kiss)");
                dlg->AddFilter("", "All files");
                dlg->SetOnCancel([this]() { this->CloseDialog(); });
                dlg->SetOnDone([this](const char *path) {
                    this->m_cbExportParams.call((void*)path);
                    this->CloseDialog();
                });
                ShowDialog(dlg);
            }

        } // namespace visualizer
    }     // namespace visualization
} // namespace open3d

#endif