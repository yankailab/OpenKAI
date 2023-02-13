#ifdef USE_OPEN3D
#include "PCselectUI.h"

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

            PCselectUI::PCselectUI(const string &title, int width, int height)
                : O3DUI(title, width, height)
            {
                Init();
                Application::GetInstance().SetMenubar(NULL);
            }

            PCselectUI::~PCselectUI() {}

            void PCselectUI::Init(void)
            {
                this->O3DUI::Init();

                m_bScanning = false;
                m_bCamAuto = false;
                m_areaName = "PCAREA";

                m_sVertex = make_shared<O3DVisualizerSelections>(*m_pScene);
                m_pScene->SetOnPointsPicked(
                    [this](const std::map<
                               string,
                               vector<pair<size_t, Vector3d>>>
                               &indices,
                           int keymods) {
                        if (keymods & int(KeyModifier::SHIFT))
                        {
                            m_sVertex->UnselectIndices(indices);
                        }
                        else
                        {
                            m_sVertex->SelectIndices(indices);
                        }

                        UpdateSelectedPointSize();
                        UpdateArea();
                    });
                m_pScene->SetOnCameraChanged(
                    [this](Camera *pC) {
                        UpdateSelectedPointSize();
                    });

                InitCtrlPanel();
                UpdateUIstate();
                SetMouseCameraMode();
            }

            void PCselectUI::camMove(Vector3f vM)
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

            void PCselectUI::UpdateUIstate(void)
            {
                m_panelCtrl->SetVisible(m_uiState.m_bShowPanel);
                UpdateBtnState();

                this->O3DUI::UpdateUIstate();
            }

            void PCselectUI::UpdateBtnState(void)
            {
                m_btnScan->SetOn(m_bScanning);
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
                m_btnNewVertexSet->SetEnabled(!m_bScanning);
                m_btnDeleteVertexSet->SetEnabled(!m_bScanning);
                m_listVertexSet->SetEnabled(!m_bScanning);
                m_sliderVsize->SetEnabled(!m_bScanning);
                m_btnHiddenRemove->SetEnabled(!m_bScanning);
                m_btnFilterReset->SetEnabled(!m_bScanning);

                if (m_bScanning)
                {
                    m_btnScan->SetText("        Stop        ");
                    m_labelArea->SetText("Area not selected");
                    SetMouseCameraMode();
                }
                else
                {
                    m_btnScan->SetText("        Start        ");
                }

                m_pScene->ForceRedraw();
            }

            void PCselectUI::SetSelectedPointSize(double px)
            {
                IF_(!m_sVertex->GetNumberOfSets());
                m_sVertex->SetPointSize(px);
                m_pScene->ForceRedraw();
            }

            void PCselectUI::UpdateSelectedPointSize(void)
            {
                int iS = m_listVertexSet->GetSelectedIndex();
                size_t nSet = m_sVertex->GetNumberOfSets();
                if (iS < 0 || iS >= nSet)
                    return;

                std::map<string, set<O3DVisualizerSelections::SelectedIndex>> msSI;
                msSI = m_sVertex->GetSets().at(iS);
                set<O3DVisualizerSelections::SelectedIndex> sSI = msSI[m_modelName];
                int nP = sSI.size();

                if (nP <= 0)
                    return;

                //update selected point size based on its pos related to camera
                m_uiState.m_vCamPos = m_pScene->GetScene()->GetCamera()->GetPosition();
                float dAvr = 0.0;
                for (O3DVisualizerSelections::SelectedIndex s : sSI)
                {
                    dAvr += (m_uiState.m_vCamPos - s.point.cast<float>()).norm();
                }
                dAvr /= (float)nP;
                SetSelectedPointSize(constrain(dAvr / 100.0, 0.025, 100.0));
            }


            void PCselectUI::SetMouseCameraMode(void)
            {
                if (m_sVertex->IsActive() && m_sVertex->GetNumberOfSets() > 0)
                    m_sVertex->MakeInactive();

                m_pScene->SetViewControls(m_uiState.m_mouseMode);
                m_uiMode = uiMode_cam;
            }

            void PCselectUI::SetMousePickingMode(void)
            {
                if (m_sVertex->GetNumberOfSets() > 0)
                    m_sVertex->MakeActive();

                m_pScene->SetViewControls(SceneWidget::Controls::PICK_POINTS);
                m_uiMode = uiMode_pointPick;
            }

            vector<O3DVisualizerSelections::SelectionSet> PCselectUI::GetSelectionSets() const
            {
                return m_sVertex->GetSets();
            }

            void PCselectUI::SetProgressBar(float v)
            {
                m_progScan->SetValue(v);
                string s = "Memory used: " + i2str((int)(v * 100)) + "%";
                m_labelProg->SetText(s.c_str());
            }

            void PCselectUI::SetLabelArea(const string &s)
            {
                m_labelArea->SetText(s.c_str());
            }

            void PCselectUI::SetCbScan(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbScan.add(pCb, pPCV);
            }

            void PCselectUI::SetCbOpenPC(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbOpenPC.add(pCb, pPCV);
            }

            void PCselectUI::SetCbCamSet(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbCamSet.add(pCb, pPCV);
            }

            void PCselectUI::SetCbVoxelDown(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbVoxelDown.add(pCb, pPCV);
            }

            void PCselectUI::SetCbHiddenRemove(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbHiddenRemove.add(pCb, pPCV);
            }

            void PCselectUI::SetCbResetPC(OnCbO3DUI pCb, void *pPCV)
            {
                m_cbResetPC.add(pCb, pPCV);
            }


            void PCselectUI::Layout(const gui::LayoutContext &context)
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

            void PCselectUI::InitCtrlPanel(void)
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
                m_btnOpenPC->SetOnClicked([this]() {
                    OnOpenPLY();
                });

                m_btnSavePC = new Button(" Save ");
                m_btnSavePC->SetOnClicked([this]() {
                    OnSavePLY();
                });

                m_btnSaveRGB = new Button("Screen");
                m_btnSaveRGB->SetOnClicked([this]() {
                    OnSaveRGB();
                });

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
                m_btnCamAuto->SetOnClicked([this]() {
                    m_bCamAuto = !m_bCamAuto;
                    int m = m_bCamAuto ? 1 : 0;
                    m_cbCamSet.call(&m);
                    UpdateBtnState();
                    m_pScene->ForceRedraw();
                });

                m_btnCamAll = new Button("    All    ");
                m_btnCamAll->SetOnClicked([this]() {
                    int m = 4;
                    m_cbCamSet.call(&m);
                    if (m_uiMode == uiMode_pointPick)
                        UpdateSelectableGeometry();
                    m_pScene->ForceRedraw();
                });

                m_btnCamOrigin = new Button(" Origin ");
                m_btnCamOrigin->SetOnClicked([this]() {
                    int m = 3;
                    m_cbCamSet.call(&m);
                    if (m_uiMode == uiMode_pointPick)
                        UpdateSelectableGeometry();
                    m_pScene->ForceRedraw();
                });

                m_btnCamL = new Button(" < ");
                m_btnCamL->SetOnClicked([this]() {
                    camMove(Vector3f(-1, 0, 0));
                });

                m_btnCamR = new Button(" > ");
                m_btnCamR->SetOnClicked([this]() {
                    camMove(Vector3f(1, 0, 0));
                });

                m_btnCamF = new Button(" ^ ");
                m_btnCamF->SetOnClicked([this]() {
                    camMove(Vector3f(0, 0, -1));
                });

                m_btnCamB = new Button(" v ");
                m_btnCamB->SetOnClicked([this]() {
                    camMove(Vector3f(0, 0, 1));
                });

                m_btnCamU = new Button(" Up ");
                m_btnCamU->SetOnClicked([this]() {
                    camMove(Vector3f(0, 1, 0));
                });

                m_btnCamD = new Button(" Down ");
                m_btnCamD->SetOnClicked([this]() {
                    camMove(Vector3f(0, -1, 0));
                });

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
                sliderPointSize->SetOnValueChanged([this](const double v) {
                    SetPointSize(int(v));
                });

                m_sliderVsize = new Slider(Slider::DOUBLE);
                m_sliderVsize->SetLimits(0.0, 1.0);
                m_sliderVsize->SetValue(m_uiState.m_sVoxel);
                m_sliderVsize->SetOnValueChanged([this](const double v) {
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
                m_btnHiddenRemove->SetOnClicked([this]() {
                    m_uiState.m_vCamPos = m_pScene->GetScene()->GetCamera()->GetPosition();
                    m_cbHiddenRemove.call(&m_uiState);
                    m_pScene->ForceRedraw();
                });

                m_btnFilterReset = new Button(" Reset ");
                m_btnFilterReset->SetOnClicked([this]() {
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

                m_btnScan = new Button("        Start        ");
                m_btnScan->SetToggleable(true);
                m_btnScan->SetOnClicked([this]() {
                    RemoveAllVertexSet();
                    UpdateArea();
                    m_bScanning = !m_bScanning;
                    m_cbScan.call(&m_bScanning);
                    m_bCamAuto = m_bScanning;
                    int m = m_bCamAuto ? 1 : 0;
                    m_cbCamSet.call(&m);
                    UpdateBtnState();
                    PostRedraw();
                });
                pH = new Horiz(v_spacing);
                pH->AddChild(GiveOwnership(m_btnScan));
                pH->AddStretch();
                panelScan->AddChild(GiveOwnership(pH));

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

                // Vertex selection
                auto panelVertexSet = new CollapsableVert("MEASURE", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelVertexSet));

                m_btnNewVertexSet = new SmallButton("   +   ");
                m_btnNewVertexSet->SetOnClicked(
                    [this]() {
                        UpdateSelectableGeometry();
                        NewVertexSet();
                        SetMousePickingMode();
                        UpdateArea();
                    });
                m_btnDeleteVertexSet = new SmallButton("   -   ");
                m_btnDeleteVertexSet->SetOnClicked(
                    [this]() {
                        RemoveVertexSet(m_listVertexSet->GetSelectedIndex());
                        UpdateArea();
                    });
                pH = new Horiz(v_spacing);
                pH->AddChild(make_shared<Label>("Select Area"));
                pH->AddStretch();
                pH->AddChild(GiveOwnership(m_btnNewVertexSet));
                pH->AddChild(GiveOwnership(m_btnDeleteVertexSet));
                panelVertexSet->AddChild(GiveOwnership(pH));

                pH = new Horiz(v_spacing);
                m_labelArea = new Label("Area not selected");
                pH->AddChild(GiveOwnership(m_labelArea));
                pH->AddStretch();
                panelVertexSet->AddChild(GiveOwnership(pH));

                m_listVertexSet = new ListView();
                m_listVertexSet->SetOnValueChanged(
                    [this](const char *, bool) {
                        UpdateSelectableGeometry();
                        SelectVertexSet(m_listVertexSet->GetSelectedIndex());
                        SetMousePickingMode();
                        UpdateArea();
                    });
                panelVertexSet->AddChild(GiveOwnership(m_listVertexSet));
            }

            void PCselectUI::UpdateSelectableGeometry(void)
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

            void PCselectUI::NewVertexSet(void)
            {
                m_sVertex->NewSet();
                UpdateVertexSetList();
                SelectVertexSet(m_sVertex->GetNumberOfSets() - 1);
            }

            void PCselectUI::SelectVertexSet(int i)
            {
                m_sVertex->SelectSet(i);
                m_listVertexSet->SetSelectedIndex(i);
            }

            void PCselectUI::UpdateVertexSetList(void)
            {
                size_t n = m_sVertex->GetNumberOfSets();
                vector<string> items;
                items.reserve(n);
                for (size_t i = 0; i < n; ++i)
                {
                    stringstream s;
                    s << "Area " << (i + 1);
                    items.push_back(s.str());
                }
                m_listVertexSet->SetItems(items);

                PostRedraw();
            }

            void PCselectUI::RemoveVertexSet(int i)
            {
                if (i < 0)
                    return;
                m_sVertex->RemoveSet(i);
                UpdateVertexSetList();
            }

            void PCselectUI::RemoveAllVertexSet(void)
            {
                while (m_sVertex->GetNumberOfSets() > 0)
                    m_sVertex->RemoveSet(0);

                UpdateVertexSetList();
            }

            void PCselectUI::UpdateArea(void)
            {
                RemoveGeometry(m_areaName);
                RemoveDistLabel();

                int iS = m_listVertexSet->GetSelectedIndex();
                size_t nSet = m_sVertex->GetNumberOfSets();

                //no vertex set is selecrted
                IF_(iS < 0 || iS >= nSet);

                //draw polygon
                std::map<string, set<O3DVisualizerSelections::SelectedIndex>> msSI;
                msSI = m_sVertex->GetSets().at(iS);
                set<O3DVisualizerSelections::SelectedIndex> sSI = msSI[m_modelName];
                int nP = sSI.size();

                //only one point is slected
                if (nP < 2)
                {
                    PostRedraw();
                    return;
                }

                //Make a line if more than a line is selected
                vector<O3DVisualizerSelections::SelectedIndex> vSI;
                for (O3DVisualizerSelections::SelectedIndex sI : sSI)
                {
                    int i;
                    for (i = vSI.size() - 1; i >= 0; i--)
                    {
                        if (vSI[i].order < sI.order)
                            break;
                    }

                    vSI.insert(vSI.begin() + i + 1, sI);
                }

                shared_ptr<geometry::LineSet> spLS = shared_ptr<geometry::LineSet>(new geometry::LineSet());
                for (int i = 0; i < vSI.size(); i++)
                {
                    spLS->points_.push_back(vSI[i].point);
                }

                nP = spLS->points_.size();
                for (int i = 0; i < nP; i++)
                {
                    int j = (i + 1) % nP;
                    spLS->lines_.push_back(Vector2i(i, j));
                    spLS->colors_.push_back(m_uiState.m_vAreaLineCol);
                }

                if (nP < 3)
                {
                    spLS->lines_.erase(spLS->lines_.end());
                    spLS->colors_.erase(spLS->colors_.end());
                }
                AddGeometry(m_areaName, spLS);

                //Distance labels
                Vector3d vPa(0, 0, 0);
                for (int i = 0; i < nP; i++)
                {
                    int j = (i + 1) % nP;
                    vPa += spLS->points_[i];
                    Vector3d p1 = spLS->points_[i];
                    Vector3d p2 = spLS->points_[j];
                    Vector3d vPd = (p1 + p2) * 0.5;
                    Vector3f vPdf = Vector3f(vPd.x(), vPd.y(), vPd.z());
                    double d = (p2 - p1).norm();
                    string strD = f2str(d, 3) + " m";
                    shared_ptr<Label3D> spL = m_pScene->AddLabel(vPdf, strD.c_str());
                    spL->SetPosition(vPdf);
                    spL->SetTextColor(Color(1, 1, 1));
                    m_vspDistLabel.push_back(spL);
                }

                if (nP < 3)
                {
                    m_pScene->RemoveLabel(m_vspDistLabel.back());
                    m_vspDistLabel.erase(m_vspDistLabel.end());
                }
                else
                {
                    double S = Area(spLS->points_);
                    string strS = "Area = " + f2str(S, 3) + " m^2";
                    vPa /= nP;
                    Vector3f vPaf = Vector3f(vPa.x(), vPa.y(), vPa.z());
                    shared_ptr<Label3D> spA = m_pScene->AddLabel(vPaf, strS.c_str());
                    spA->SetPosition(vPaf);
                    spA->SetTextColor(Color(1, 1, 1));
                    m_vspDistLabel.push_back(spA);

                    strS = "Area" + i2str(iS + 1) + " = " + f2str(S, 3) + " m^2";
                    SetLabelArea(strS);
                }

                PostRedraw();
            }

            void PCselectUI::RemoveDistLabel(void)
            {
                for (shared_ptr<Label3D> spLabel : m_vspDistLabel)
                {
                    m_pScene->RemoveLabel(spLabel);
                }
                m_vspDistLabel.clear();
            }

            double PCselectUI::Area(vector<Vector3d> &vP)
            {
                int nP = vP.size();
                Vector3d vA(0, 0, 0);
                int j = 0;

                for (int i = 0; i < nP; i++)
                {
                    j = (i + 1) % nP;
                    vA += vP[i].cross(vP[j]);
                }

                vA *= 0.5;
                return vA.norm();
            }

            void PCselectUI::OnSaveRGB(void)
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

            void PCselectUI::OnSavePLY(void)
            {
                auto dlg = make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::SAVE, "Save File", this->GetTheme());
                dlg->AddFilter(".ply", "Point Cloud Files (.ply)");
                dlg->AddFilter("", "All files");
                dlg->SetOnCancel([this]() { this->CloseDialog(); });
                dlg->SetOnDone([this](const char *path) {
                    this->CloseDialog();

                    ShowMsg("Save", "Saving .Ply file");
                    io::WritePointCloudOption par;
                    par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
                    par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

                    shared_ptr<t::geometry::PointCloud> spTpc = GetGeometry(m_modelName).m_sTgeometry;
                    io::WritePointCloudToPLY(path, spTpc->ToLegacy(), par);
                    CloseMsg();
                });
                ShowDialog(dlg);
            }

            void PCselectUI::OnOpenPLY(void)
            {
                auto dlg = make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::OPEN, "Save File", this->GetTheme());
                dlg->AddFilter(".ply", "Point Cloud Files (.ply)");
                dlg->AddFilter("", "All files");
                dlg->SetOnCancel([this]() { this->CloseDialog(); });
                dlg->SetOnDone([this](const char *path) {
                    this->CloseDialog();
                    this->m_cbOpenPC.call((void *)path);
                });
                ShowDialog(dlg);
            }

        } // namespace visualizer
    }     // namespace visualization
} // namespace open3d

#endif