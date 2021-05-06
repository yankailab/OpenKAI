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

                m_dV = 0.01;
                InitCtrlPanel();
                UpdateUIstate();

                m_pScene->SetViewControls(m_uiState.m_mouseMode);

                auto menu = std::make_shared<Menu>();
                auto file_menu = std::make_shared<Menu>();
                file_menu->AddItem("Load calibration images", MENU_LOAD_IMGS);
                file_menu->AddSeparator();
                file_menu->AddItem("Import params", MENU_IMPORT_PARAMS);
                file_menu->AddItem("Export params", MENU_EXPORT_PARAMS);
                file_menu->AddSeparator();
                file_menu->AddItem("Reset Pointcloud", MENU_RESET_PC);
                menu->AddMenu("File", file_menu);

                Application::GetInstance().SetMenubar(menu);
                SetOnMenuItemActivated(MENU_LOAD_IMGS,
                                       [this]() { OnLoadImgs(); });
                SetOnMenuItemActivated(MENU_IMPORT_PARAMS,
                                       [this]() { OnImportParams(); });
                SetOnMenuItemActivated(MENU_EXPORT_PARAMS,
                                       [this]() { OnExportParams(); });
                SetOnMenuItemActivated(MENU_RESET_PC,
                                       [this]() {
                                           m_cbResetPC.call();
                                       });
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
                m_pFx->SetText(f2str(m_calibParam.m_Fx).c_str());
                m_pFy->SetText(f2str(m_calibParam.m_Fy).c_str());
                m_pCx->SetText(f2str(m_calibParam.m_Cx).c_str());
                m_pCy->SetText(f2str(m_calibParam.m_Cy).c_str());

                m_poTx->SetText(f2str(m_calibParam.m_oTx).c_str());
                m_poTy->SetText(f2str(m_calibParam.m_oTy).c_str());
                m_poTz->SetText(f2str(m_calibParam.m_oTz).c_str());

                m_poRx->SetText(f2str(m_calibParam.m_oRx).c_str());
                m_poRy->SetText(f2str(m_calibParam.m_oRy).c_str());
                m_poRz->SetText(f2str(m_calibParam.m_oRz).c_str());

                m_pk1->SetText(f2str(m_calibParam.m_k1).c_str());
                m_pk2->SetText(f2str(m_calibParam.m_k2).c_str());
                m_pk3->SetText(f2str(m_calibParam.m_k3).c_str());
                m_pk4->SetText(f2str(m_calibParam.m_k4).c_str());
            }

            void PCcalibUI::SetCbLoadImgs(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbLoadImgs.add(pCb, pPCV);
            }

            void PCcalibUI::SetCbResetPC(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbResetPC.add(pCb, pPCV);
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

                // Cam Intrinsic
                auto panelCam = new CollapsableVert("Cam Intrinsic", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelCam));

                auto *pG = new VGrid(4, v_spacing);
                panelCam->AddChild(GiveOwnership(pG));

                m_pFx = new TextEdit();
                m_pFx->SetText("0.0");
                m_pFx->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_Fx = atof(m_pFx->GetText());
                });
                auto *pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pFx->SetText(f2str(atof(m_pFx->GetText()) + m_dV).c_str());
                });
                auto *pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pFx->SetText(f2str(atof(m_pFx->GetText()) - m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("Fx")));
                pG->AddChild(GiveOwnership(m_pFx));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pFy = new TextEdit();
                m_pFy->SetText("0.0");
                m_pFy->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_Fy = atof(m_pFy->GetText());
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pFy->SetText(f2str(atof(m_pFy->GetText()) + m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pFy->SetText(f2str(atof(m_pFy->GetText()) - m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("Fy")));
                pG->AddChild(GiveOwnership(m_pFy));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pCx = new TextEdit();
                m_pCx->SetText("0.0");
                m_pCx->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_Cx = atof(m_pCx->GetText());
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pCx->SetText(f2str(atof(m_pCx->GetText()) + m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pCx->SetText(f2str(atof(m_pCx->GetText()) - m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("Cx")));
                pG->AddChild(GiveOwnership(m_pCx));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pCy = new TextEdit();
                m_pCy->SetText("0.0");
                m_pCy->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_Cy = atof(m_pCy->GetText());
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pCy->SetText(f2str(atof(m_pCy->GetText()) + m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pCy->SetText(f2str(atof(m_pCy->GetText()) - m_dV).c_str());
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
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pk1->SetText(f2str(atof(m_pk1->GetText()) + m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pk1->SetText(f2str(atof(m_pk1->GetText()) - m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("k1")));
                pG->AddChild(GiveOwnership(m_pk1));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pk2 = new TextEdit();
                m_pk2->SetText("0.0");
                m_pk2->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_k2 = atof(m_pk2->GetText());
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pk2->SetText(f2str(atof(m_pk2->GetText()) + m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pk2->SetText(f2str(atof(m_pk2->GetText()) - m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("k2")));
                pG->AddChild(GiveOwnership(m_pk2));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pk3 = new TextEdit();
                m_pk3->SetText("0.0");
                m_pk3->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_k3 = atof(m_pk3->GetText());
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pk3->SetText(f2str(atof(m_pk3->GetText()) + m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pk3->SetText(f2str(atof(m_pk3->GetText()) - m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("k3")));
                pG->AddChild(GiveOwnership(m_pk3));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pk4 = new TextEdit();
                m_pk4->SetText("0.0");
                m_pk4->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_k4 = atof(m_pk4->GetText());
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pk4->SetText(f2str(atof(m_pk4->GetText()) + m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pk4->SetText(f2str(atof(m_pk4->GetText()) - m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("k4")));
                pG->AddChild(GiveOwnership(m_pk4));
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
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_poTx->SetText(f2str(atof(m_poTx->GetText()) + m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_poTx->SetText(f2str(atof(m_poTx->GetText()) - m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("tX")));
                pG->AddChild(GiveOwnership(m_poTx));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_poTy = new TextEdit();
                m_poTy->SetText("0.0");
                m_poTy->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_oTy = atof(m_poTy->GetText());
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_poTy->SetText(f2str(atof(m_poTy->GetText()) + m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_poTy->SetText(f2str(atof(m_poTy->GetText()) - m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("tY")));
                pG->AddChild(GiveOwnership(m_poTy));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_poTz = new TextEdit();
                m_poTz->SetText("0.0");
                m_poTz->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_oTz = atof(m_poTz->GetText());
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_poTz->SetText(f2str(atof(m_poTz->GetText()) + m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_poTz->SetText(f2str(atof(m_poTz->GetText()) - m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("tZ")));
                pG->AddChild(GiveOwnership(m_poTz));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_poRx = new TextEdit();
                m_poRx->SetText("0.0");
                m_poRx->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_oRx = atof(m_poRx->GetText());
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_poRx->SetText(f2str(atof(m_poRx->GetText()) + m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_poRx->SetText(f2str(atof(m_poRx->GetText()) - m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("rX")));
                pG->AddChild(GiveOwnership(m_poRx));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_poRy = new TextEdit();
                m_poRy->SetText("0.0");
                m_poRy->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_oRy = atof(m_poRy->GetText());
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_poRy->SetText(f2str(atof(m_poRy->GetText()) + m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_poRy->SetText(f2str(atof(m_poRy->GetText()) - m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("rY")));
                pG->AddChild(GiveOwnership(m_poRy));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_poRz = new TextEdit();
                m_poRz->SetText("0.0");
                m_poRz->SetOnTextChanged([this](const char *) {
                    m_calibParam.m_oRz = atof(m_poRz->GetText());
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_poRz->SetText(f2str(atof(m_poRz->GetText()) + m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_poRz->SetText(f2str(atof(m_poRz->GetText()) - m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("rZ")));
                pG->AddChild(GiveOwnership(m_poRz));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));
            }

            void PCcalibUI::OnLoadImgs(void)
            {
                auto dlg = make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::OPEN, "Open Directory", this->GetTheme());
                dlg->AddFilter("", "All files");
                dlg->SetOnCancel([this]() { this->CloseDialog(); });
                dlg->SetOnDone([this](const char *path) {
                    this->CloseDialog();
                    m_cbLoadImgs.call((void*)path);
                });
                ShowDialog(dlg);
            }

            void PCcalibUI::OnImportParams(void)
            {
                auto dlg = make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::OPEN, "Open Calibration File", this->GetTheme());
                dlg->AddFilter(".kiss", "Calibration file (.kiss)");
                dlg->AddFilter("", "All files");
                dlg->SetOnCancel([this]() { this->CloseDialog(); });
                dlg->SetOnDone([this](const char *path) {
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
                    this->CloseDialog();

                    
                });
                ShowDialog(dlg);
            }

        } // namespace visualizer
    }     // namespace visualization
} // namespace open3d

#endif