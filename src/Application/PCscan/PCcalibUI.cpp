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
                Application::GetInstance().SetMenubar(NULL);
            }

            PCcalibUI::~PCcalibUI() {}

            void PCcalibUI::Init(void)
            {
                this->O3DUI::Init();

                m_dV = 0.01;

                InitCtrlPanel();
                UpdateUIstate();

                m_pScene->SetViewControls(m_uiState.m_mouseMode);
            }

            void PCcalibUI::UpdateUIstate(void)
            {
                m_panelCtrl->SetVisible(m_uiState.m_bShowPanel);
                UpdateBtnState();

                this->O3DUI::UpdateUIstate();
            }

            void PCcalibUI::UpdateBtnState(void)
            {
                m_pScene->ForceRedraw();
            }

            void PCcalibUI::SetCbBtnCalib(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbBtnScan.add(pCb, pPCV);
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

                auto btnLoadImgs = new Button(" Load Calib Images ");
                btnLoadImgs->SetOnClicked([this]() {
                });

                auto pH = new Horiz(v_spacing);
                pH->AddChild(GiveOwnership(btnLoadImgs));
                pH->AddStretch();
                panelCam->AddChild(GiveOwnership(pH));

                auto *pG = new VGrid(4, v_spacing);
                
                m_pFx = new TextEdit();
                m_pFx->SetText("0.0");
                m_pFx->SetOnTextChanged([this](const char*) {
                });
                auto* pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pFx->SetText(f2str(atof(m_pFx->GetText())+m_dV).c_str());
                });
                auto* pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pFx->SetText(f2str(atof(m_pFx->GetText())-m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("Fx")));
                pG->AddChild(GiveOwnership(m_pFx));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pFy = new TextEdit();
                m_pFy->SetText("0.0");
                m_pFy->SetOnTextChanged([this](const char*) {
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pFy->SetText(f2str(atof(m_pFy->GetText())+m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pFy->SetText(f2str(atof(m_pFy->GetText())-m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("Fy")));
                pG->AddChild(GiveOwnership(m_pFy));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pCx = new TextEdit();
                m_pCx->SetText("0.0");
                m_pCx->SetOnTextChanged([this](const char*) {
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pCx->SetText(f2str(atof(m_pCx->GetText())+m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pCx->SetText(f2str(atof(m_pCx->GetText())-m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("Cx")));
                pG->AddChild(GiveOwnership(m_pCx));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                m_pCy = new TextEdit();
                m_pCy->SetText("0.0");
                m_pCy->SetOnTextChanged([this](const char*) {
                });
                pEinc = new SmallButton(" + ");
                pEinc->SetOnClicked([this]() {
                    m_pCy->SetText(f2str(atof(m_pCy->GetText())+m_dV).c_str());
                });
                pEdec = new SmallButton(" - ");
                pEdec->SetOnClicked([this]() {
                    m_pCy->SetText(f2str(atof(m_pCy->GetText())-m_dV).c_str());
                });
                pG->AddChild(GiveOwnership(new Label("Cy")));
                pG->AddChild(GiveOwnership(m_pCy));
                pG->AddChild(GiveOwnership(pEinc));
                pG->AddChild(GiveOwnership(pEdec));

                panelCam->AddChild(GiveOwnership(pG));

            }

            void PCcalibUI::OnSaveData(void)
            {
                auto dlg = make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::SAVE, "Save File", this->GetTheme());
                dlg->AddFilter(".png", "PNG images (.png)");
                dlg->AddFilter("", "All files");
                dlg->SetOnCancel([this]() { this->CloseDialog(); });
                dlg->SetOnDone([this](const char *path) {
                    this->CloseDialog();
                    this->ExportCurrentImage(path);
                });
                ShowDialog(dlg);
            }

        } // namespace visualizer
    }     // namespace visualization
} // namespace open3d

#endif