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
                return &m_calibParam;
            }

            void PCcalibUI::UpdateCalibParams(void)
            {
                m_pFx->SetText(lf2str(m_calibParam.m_Fx,m_nD).c_str());
                m_pFy->SetText(lf2str(m_calibParam.m_Fy,m_nD).c_str());
                m_pCx->SetText(lf2str(m_calibParam.m_Cx,m_nD).c_str());
                m_pCy->SetText(lf2str(m_calibParam.m_Cy,m_nD).c_str());

                m_poTx->SetText(lf2str(m_calibParam.m_oTx,m_nD).c_str());
                m_poTy->SetText(lf2str(m_calibParam.m_oTy,m_nD).c_str());
                m_poTz->SetText(lf2str(m_calibParam.m_oTz,m_nD).c_str());

                m_poRx->SetText(lf2str(m_calibParam.m_oRx,m_nD).c_str());
                m_poRy->SetText(lf2str(m_calibParam.m_oRy,m_nD).c_str());
                m_poRz->SetText(lf2str(m_calibParam.m_oRz,m_nD).c_str());

                m_pk1->SetText(lf2str(m_calibParam.m_k1,m_nD).c_str());
                m_pk2->SetText(lf2str(m_calibParam.m_k2,m_nD).c_str());
                m_pp1->SetText(lf2str(m_calibParam.m_p1,m_nD).c_str());
                m_pp2->SetText(lf2str(m_calibParam.m_p2,m_nD).c_str());
                m_pk3->SetText(lf2str(m_calibParam.m_k3,m_nD).c_str());
            }

            void PCcalibUI::SetCbScan(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbScan.add(pCb, pPCV);
            }

            void PCcalibUI::SetCbResetPC(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbResetPC.add(pCb, pPCV);
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

            void PCcalibUI::Layout(const Theme &theme)
            {
                int settings_width = m_uiState.m_wPanel * theme.font_size;

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

                gui::Window::Layout(theme);
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
                pH->AddStretch();

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

                auto *pG = new VGrid(4, v_spacing);
                panelCam->AddChild(GiveOwnership(pG));

                m_pFx = new TextEdit();
                m_pFx->SetText("0.0");
                m_pFx->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_Fx = atof(m_pFx->GetText());
                    m_cbUpdateParams.call();
                });
                auto *pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pFx->SetText(lf2str(atof(m_pFx->GetText()) + m_dV,m_nD).c_str());
                });
                auto *pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pFx->SetText(lf2str(atof(m_pFx->GetText()) - m_dV,m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("Fx")));
                pG->AddChild(GiveOwnership(m_pFx));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pFy = new TextEdit();
                m_pFy->SetText("0.0");
                m_pFy->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_Fy = atof(m_pFy->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pFy->SetText(lf2str(atof(m_pFy->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pFy->SetText(lf2str(atof(m_pFy->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("Fy")));
                pG->AddChild(GiveOwnership(m_pFy));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pCx = new TextEdit();
                m_pCx->SetText("0.0");
                m_pCx->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_Cx = atof(m_pCx->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pCx->SetText(lf2str(atof(m_pCx->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pCx->SetText(lf2str(atof(m_pCx->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("Cx")));
                pG->AddChild(GiveOwnership(m_pCx));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pCy = new TextEdit();
                m_pCy->SetText("0.0");
                m_pCy->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_Cy = atof(m_pCy->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pCy->SetText(lf2str(atof(m_pCy->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pCy->SetText(lf2str(atof(m_pCy->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("Cy")));
                pG->AddChild(GiveOwnership(m_pCy));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                // Cam distortion
                auto panelDistort = new CollapsableVert("Cam Distortion", v_spacing, margins);
                panelDistort->SetIsOpen(false);
                m_panelCtrl->AddChild(GiveOwnership(panelDistort));
                pG = new VGrid(4, v_spacing);
                panelDistort->AddChild(GiveOwnership(pG));

                m_pk1 = new TextEdit();
                m_pk1->SetText("0.0");
                m_pk1->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_k1 = atof(m_pk1->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pk1->SetText(lf2str(atof(m_pk1->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pk1->SetText(lf2str(atof(m_pk1->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("k1")));
                pG->AddChild(GiveOwnership(m_pk1));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pk2 = new TextEdit();
                m_pk2->SetText("0.0");
                m_pk2->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_k2 = atof(m_pk2->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pk2->SetText(lf2str(atof(m_pk2->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pk2->SetText(lf2str(atof(m_pk2->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("k2")));
                pG->AddChild(GiveOwnership(m_pk2));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pp1 = new TextEdit();
                m_pp1->SetText("0.0");
                m_pp1->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_p1 = atof(m_pp1->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pp1->SetText(lf2str(atof(m_pp1->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pp1->SetText(lf2str(atof(m_pp1->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("p1")));
                pG->AddChild(GiveOwnership(m_pp1));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pp2 = new TextEdit();
                m_pp2->SetText("0.0");
                m_pp2->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_p2 = atof(m_pp2->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pp2->SetText(lf2str(atof(m_pp2->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pp2->SetText(lf2str(atof(m_pp2->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("p2")));
                pG->AddChild(GiveOwnership(m_pp2));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pk3 = new TextEdit();
                m_pk3->SetText("0.0");
                m_pk3->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_k3 = atof(m_pk3->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pk3->SetText(lf2str(atof(m_pk3->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pk3->SetText(lf2str(atof(m_pk3->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("k3")));
                pG->AddChild(GiveOwnership(m_pk3));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));


                // Sensor offset
                auto panelOffset = new CollapsableVert("Sensor Offset", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelOffset));

                pG = new VGrid(4, v_spacing);
                panelOffset->AddChild(GiveOwnership(pG));

                m_poTx = new TextEdit();
                m_poTx->SetText("0.0");
                m_poTx->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_oTx = atof(m_poTx->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_poTx->SetText(lf2str(atof(m_poTx->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_poTx->SetText(lf2str(atof(m_poTx->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("tX")));
                pG->AddChild(GiveOwnership(m_poTx));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_poTy = new TextEdit();
                m_poTy->SetText("0.0");
                m_poTy->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_oTy = atof(m_poTy->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_poTy->SetText(lf2str(atof(m_poTy->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_poTy->SetText(lf2str(atof(m_poTy->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("tY")));
                pG->AddChild(GiveOwnership(m_poTy));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_poTz = new TextEdit();
                m_poTz->SetText("0.0");
                m_poTz->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_oTz = atof(m_poTz->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_poTz->SetText(lf2str(atof(m_poTz->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_poTz->SetText(lf2str(atof(m_poTz->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("tZ")));
                pG->AddChild(GiveOwnership(m_poTz));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_poRx = new TextEdit();
                m_poRx->SetText("0.0");
                m_poRx->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_oRx = atof(m_poRx->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_poRx->SetText(lf2str(atof(m_poRx->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_poRx->SetText(lf2str(atof(m_poRx->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("rX")));
                pG->AddChild(GiveOwnership(m_poRx));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_poRy = new TextEdit();
                m_poRy->SetText("0.0");
                m_poRy->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_oRy = atof(m_poRy->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_poRy->SetText(lf2str(atof(m_poRy->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_poRy->SetText(lf2str(atof(m_poRy->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("rY")));
                pG->AddChild(GiveOwnership(m_poRy));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_poRz = new TextEdit();
                m_poRz->SetText("0.0");
                m_poRz->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_oRz = atof(m_poRz->GetText());
                    m_cbUpdateParams.call();
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_poRz->SetText(lf2str(atof(m_poRz->GetText()) + m_dV, m_nD).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_poRz->SetText(lf2str(atof(m_poRz->GetText()) - m_dV, m_nD).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("rZ")));
                pG->AddChild(GiveOwnership(m_poRz));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));
            }

            void PCcalibUI::OnImportParams(void)
            {
                auto dlg = make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::OPEN, "Open Calibration File", this->GetTheme());
                dlg->AddFilter(".yml", "Camera calibration file (.yml)");
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
                dlg->AddFilter(".yml", "Camera calibration file (.yml)");
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