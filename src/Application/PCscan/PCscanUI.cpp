#ifdef USE_OPEN3D
#include "PCscanUI.h"

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

            PCscanUI::PCscanUI(const string &title, int width, int height)
                : Window(title, width, height)
            {
                Init();
                Application::GetInstance().SetMenubar(NULL);
            }

            PCscanUI::~PCscanUI() {}

            void PCscanUI::Init(void)
            {
                if (m_pWindow)
                    return;

                m_bScanning = false;
                m_bCamAuto = false;
                m_modelName = "PCMODEL";
                m_areaName = "PCAREA";

                m_pWindow = this;
                m_pScene = new SceneWidget();
                m_sVertex = make_shared<O3DVisualizerSelections>(*m_pScene);
                m_pScene->SetScene(make_shared<Open3DScene>(m_pWindow->GetRenderer()));
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

                        UpdateArea();
                    });
                m_pWindow->AddChild(GiveOwnership(m_pScene));

                m_pScene->SetOnCameraChanged(
                    [this](Camera *pC) {
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
                        m_uiState.m_vCamPos = pC->GetPosition();
                        float dAvr = 0.0;
                        for (O3DVisualizerSelections::SelectedIndex s : sSI)
                        {
                            dAvr += (m_uiState.m_vCamPos - s.point.cast<float>()).norm();
                        }
                        dAvr /= (float)nP;
                        SetSelectedPointSize(constrain(dAvr / 100.0, 0.025, 100.0));
                    });

                InitCtrlPanel();
                UpdateUIstate();
                SetMouseCameraMode();
            }

            void PCscanUI::AddGeometry(const string &name,
                                       shared_ptr<geometry::Geometry3D> spG,
                                       rendering::Material *material,
                                       bool bVisible)
            {
                bool is_default_color;
                bool no_shadows = false;
                Material mat;
                if (material)
                {
                    mat = *material;
                    is_default_color = false;
                }
                else
                {
                    bool has_colors = false;
                    bool has_normals = false;

                    auto cloud = dynamic_pointer_cast<geometry::PointCloud>(spG);
                    auto lines = dynamic_pointer_cast<geometry::LineSet>(spG);
                    auto mesh = dynamic_pointer_cast<geometry::MeshBase>(spG);

                    if (cloud)
                    {
                        has_colors = !cloud->colors_.empty();
                        has_normals = !cloud->normals_.empty();
                    }
                    else if (lines)
                    {
                        has_colors = !lines->colors_.empty();
                        no_shadows = true;
                    }
                    else if (mesh)
                    {
                        has_normals = !mesh->vertex_normals_.empty();
                        has_colors = true; // always want base_color as white
                    }

                    mat.base_color = {1.0f, 1.0f, 1.0f, 1.0f};
                    mat.shader = "defaultUnlit";
                    if (lines)
                    {
                        mat.shader = "unlitLine";
                        mat.line_width = m_uiState.m_wLine * m_pWindow->GetScaling();
                    }
                    is_default_color = true;
                    if (has_colors)
                    {
                        is_default_color = false;
                    }
                    if (has_normals)
                    {
                        mat.shader = "defaultLit";
                        is_default_color = false;
                    }
                    mat.point_size = ConvertToScaledPixels(m_uiState.m_sPoint);
                }

                m_vObject.push_back({name, spG, nullptr, mat, bVisible});

                auto scene = m_pScene->GetScene();
                scene->AddGeometry(name, spG.get(), mat);
                scene->GetScene()->GeometryShadows(name, false, false);
                scene->ShowGeometry(name, bVisible);

                SetPointSize(m_uiState.m_sPoint);
                m_pScene->ForceRedraw();
            }

            void PCscanUI::AddPointCloud(const string &name,
                                         shared_ptr<t::geometry::PointCloud> sTg,
                                         rendering::Material *material,
                                         bool bVisible)
            {
                bool no_shadows = false;
                Material mat;
                bool has_colors = false;
                bool has_normals = false;

                auto t_cloud = dynamic_pointer_cast<t::geometry::PointCloud>(sTg);
                auto t_mesh = dynamic_pointer_cast<t::geometry::TriangleMesh>(sTg);
                if (t_cloud)
                {
                    has_colors = t_cloud->HasPointColors();
                    has_normals = t_cloud->HasPointNormals();
                }
                else if (t_mesh)
                {
                    has_normals = !t_mesh->HasVertexNormals();
                    has_colors = true; // always want base_color as white
                }

                mat.base_color = {1.0f, 1.0f, 1.0f, 1.0f};
                mat.shader = "defaultUnlit";
                mat.point_size = ConvertToScaledPixels(m_uiState.m_sPoint);

                m_vObject.push_back({name, nullptr, sTg, mat, bVisible});
                auto scene = m_pScene->GetScene();

                scene->AddGeometry(name, sTg.get(), mat, false);
                scene->GetScene()->GeometryShadows(name, false, false);
                scene->ShowGeometry(name, bVisible);

                SetPointSize(m_uiState.m_sPoint);
                m_pScene->ForceRedraw();
            }

            void PCscanUI::UpdatePointCloud(const string &name,
                                            shared_ptr<t::geometry::PointCloud> sTg)
            {
                UpdateTgeometry(name, sTg);

                gui::Application::GetInstance().PostToMainThread(
                    this, [this, name]() {
                        m_pScene->GetScene()->GetScene()->UpdateGeometry(
                            name,
                            *GetGeometry(name).m_sTgeometry,
                            open3d::visualization::rendering::Scene::kUpdatePointsFlag |
                                open3d::visualization::rendering::Scene::kUpdateColorsFlag);

                        m_pScene->ForceRedraw();
                    });
            }

            void PCscanUI::RemoveGeometry(const string &name)
            {
                m_pScene->GetScene()->RemoveGeometry(name);

                for (size_t i = 0; i < m_vObject.size(); i++)
                {
                    DrawObject *pO = &m_vObject[i];
                    if (pO->m_name != name)
                        continue;

                    m_vObject.erase(m_vObject.begin() + i);
                    break;
                }

                // Bounds have changed, so update the selection point size, since they
                // depend on the bounds.
                SetPointSize(m_uiState.m_sPoint);

                m_pScene->ForceRedraw();
            }

            DrawObject PCscanUI::GetGeometry(const string &name) const
            {
                for (auto &o : m_vObject)
                {
                    if (o.m_name == name)
                        return o;
                }
                return DrawObject();
            }

            void PCscanUI::CamSetProj(
                double fov,
                double aspect,
                double near,
                double far,
                uint8_t fov_type)
            {
                auto sCam = m_pScene->GetScene()->GetCamera();
                sCam->SetProjection(
                    fov,
                    aspect,
                    near,
                    far,
                    (fov_type == 0) ? Camera::FovType::Horizontal : Camera::FovType::Vertical);
                m_pScene->ForceRedraw();
            }

            void PCscanUI::CamSetPose(
                const Vector3f &center,
                const Vector3f &eye,
                const Vector3f &up)
            {
                auto sCam = m_pScene->GetScene()->GetCamera();
                sCam->LookAt(center, eye, up);
                m_pScene->ForceRedraw();
            }

            void PCscanUI::CamAutoBound(const geometry::AxisAlignedBoundingBox &aabb,
                                        const Vector3f &CoR)
            {
                m_pScene->SetupCamera(m_pScene->GetScene()->GetCamera()->GetFieldOfView(),
                                      aabb,
                                      CoR);
                m_pScene->ForceRedraw();
            }

            void PCscanUI::camMove(Vector3f vM)
            {
                auto pC = m_pScene->GetScene()->GetCamera();
                auto mm = pC->GetModelMatrix();
                mm.translate(vM);
                pC->SetModelMatrix(mm);
                if (m_uiMode == uiMode_pointPick)
                    UpdateSelectableGeometry();

                m_pScene->ForceRedraw();
            }

            UIState *PCscanUI::getUIState(void)
            {
                return &m_uiState;
            }

            void PCscanUI::UpdateUIstate(void)
            {
                m_pScene->EnableSceneCaching(m_uiState.m_bSceneCache);
                m_panelCtrl->SetVisible(m_uiState.m_bShowPanel);
                auto pO3DScene = m_pScene->GetScene();
                pO3DScene->ShowAxes(m_uiState.m_bShowAxes);
                pO3DScene->SetBackground(m_uiState.m_vBgCol, nullptr);
                pO3DScene->SetLighting(Open3DScene::LightingProfile::NO_SHADOWS, m_uiState.m_vSunDir);
                SetPointSize(m_uiState.m_sPoint);
                SetLineWidth(m_uiState.m_wLine);
                UpdateBtnState();

                m_pWindow->SetNeedsLayout();
                m_pScene->ForceRedraw();
            }

            void PCscanUI::UpdateBtnState(void)
            {
                m_btnScanStart->SetOn(m_bScanning);
                m_btnCamAuto->SetOn(m_bCamAuto);
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
                    m_btnScanStart->SetText("        Stop        ");
                    m_labelArea->SetText("Area not selected");
                    SetMouseCameraMode();
                }
                else
                {
                    m_btnScanStart->SetText("        Start        ");
                }

                m_pScene->ForceRedraw();
            }

            void PCscanUI::SetPointSize(int px)
            {
                m_uiState.m_sPoint = px;
                px = int(ConvertToScaledPixels(px));
                for (auto &o : m_vObject)
                {
                    o.m_material.point_size = float(px);
                    m_pScene->GetScene()->GetScene()->OverrideMaterial(o.m_name, o.m_material);
                }
                m_pScene->SetPickablePointSize(px);

                m_pScene->ForceRedraw();
            }

            void PCscanUI::SetSelectedPointSize(double px)
            {
                IF_(!m_sVertex->GetNumberOfSets());
                m_sVertex->SetPointSize(px);
                m_pScene->ForceRedraw();
            }

            void PCscanUI::SetLineWidth(int px)
            {
                m_uiState.m_wLine = px;

                px = int(ConvertToScaledPixels(px));
                for (auto &o : m_vObject)
                {
                    o.m_material.line_width = float(px);
                    m_pScene->GetScene()->GetScene()->OverrideMaterial(o.m_name, o.m_material);
                }
                m_pScene->ForceRedraw();
            }

            void PCscanUI::SetMouseCameraMode(void)
            {
                if (m_sVertex->IsActive() && m_sVertex->GetNumberOfSets() > 0)
                    m_sVertex->MakeInactive();

                m_pScene->SetViewControls(m_uiState.m_mouseMode);
                CamAutoBound(m_pScene->GetScene()->GetBoundingBox(), {0.0f, 0.0f, 0.0f});
                m_uiMode = uiMode_cam;
            }

            void PCscanUI::SetMousePickingMode(void)
            {
                if (m_sVertex->GetNumberOfSets() > 0)
                    m_sVertex->MakeActive();

                m_pScene->SetViewControls(SceneWidget::Controls::PICK_POINTS);
                m_uiMode = uiMode_pointPick;
            }

            void PCscanUI::SetProgressBar(float v)
            {
                m_progScan->SetValue(v);
                string s = "Memory used: " + i2str((int)(v * 100)) + "%";
                m_labelProg->SetText(s.c_str());
            }

            void PCscanUI::SetLabelArea(const string &s)
            {
                m_labelArea->SetText(s.c_str());
            }

            void PCscanUI::SetCbBtnScan(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbBtnScan.add(pCb, pPCV);
            }

            void PCscanUI::SetCbBtnOpenPC(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbBtnOpenPC.add(pCb, pPCV);
            }

            void PCscanUI::SetCbBtnCamSet(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbBtnCamSet.add(pCb, pPCV);
            }

            void PCscanUI::SetCbBtnHiddenRemove(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbBtnHiddenRemove.add(pCb, pPCV);
            }

            void PCscanUI::SetCbBtnResetPC(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbBtnResetPC.add(pCb, pPCV);
            }

            void PCscanUI::SetCbVoxelDown(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbVoxelDown.add(pCb, pPCV);
            }

            vector<O3DVisualizerSelections::SelectionSet> PCscanUI::GetSelectionSets() const
            {
                return m_sVertex->GetSets();
            }

            void PCscanUI::ExportCurrentImage(const string &path)
            {
                m_pScene->EnableSceneCaching(false);
                m_pScene->GetScene()->GetScene()->RenderToImage(
                    [this, path](shared_ptr<geometry::Image> image) mutable {
                        if (!io::WriteImage(path, *image))
                        {
                            this->m_pWindow->ShowMessageBox(
                                "Error",
                                (string("Could not write image to ") + path + ".").c_str());
                        }
                        m_pScene->EnableSceneCaching(m_uiState.m_bSceneCache);
                    });
            }

            void PCscanUI::Layout(const Theme &theme)
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

                Super::Layout(theme);
            }

            void PCscanUI::InitCtrlPanel(void)
            {
                auto em = m_pWindow->GetTheme().font_size;
                auto half_em = int(round(0.5f * float(em)));
                auto v_spacing = int(round(0.25 * float(em)));

                m_panelCtrl = new Vert(half_em);
                m_pWindow->AddChild(GiveOwnership(m_panelCtrl));

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
                    m_cbBtnCamSet.call(&m);
                    UpdateBtnState();
                    m_pScene->ForceRedraw();
                });

                m_btnCamAll = new Button("    All    ");
                m_btnCamAll->SetOnClicked([this]() {
                    int m = 4;
                    m_cbBtnCamSet.call(&m);
                    if (m_uiMode == uiMode_pointPick)
                        UpdateSelectableGeometry();
                    m_pScene->ForceRedraw();
                });

                m_btnCamOrigin = new Button(" Origin ");
                m_btnCamOrigin->SetOnClicked([this]() {
                    int m = 3;
                    m_cbBtnCamSet.call(&m);
                    if (m_uiMode == uiMode_pointPick)
                        UpdateSelectableGeometry();
                    m_pScene->ForceRedraw();
                });

                m_btnCamL = new Button(" < ");
                m_btnCamL->SetOnClicked([this]() {
                    camMove(Vector3f(-m_uiState.m_dMove, 0, 0));
                });

                m_btnCamR = new Button(" > ");
                m_btnCamR->SetOnClicked([this]() {
                    camMove(Vector3f(m_uiState.m_dMove, 0, 0));
                });

                m_btnCamF = new Button(" ^ ");
                m_btnCamF->SetOnClicked([this]() {
                    camMove(Vector3f(0, 0, -m_uiState.m_dMove));
                });

                m_btnCamB = new Button(" v ");
                m_btnCamB->SetOnClicked([this]() {
                    camMove(Vector3f(0, 0, m_uiState.m_dMove));
                });

                m_btnCamU = new Button(" Up ");
                m_btnCamU->SetOnClicked([this]() {
                    camMove(Vector3f(0, m_uiState.m_dMove, 0));
                });

                m_btnCamD = new Button(" Down ");
                m_btnCamD->SetOnClicked([this]() {
                    camMove(Vector3f(0, -m_uiState.m_dMove, 0));
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

                // m_sliderORemovN = new Slider(Slider::INT);
                // m_sliderORemovN->SetLimits(0, 20);
                // m_sliderORemovN->SetValue(m_uiState.m_oRemovN);
                // m_sliderORemovN->SetOnValueChanged([this](const double v) {
                //      m_uiState.m_oRemovN = v;
                //     // m_cbVoxelDown.call(&m_uiState);
                //     m_pScene->ForceRedraw();
                // });

                // m_sliderORemovD = new Slider(Slider::DOUBLE);
                // m_sliderORemovD->SetLimits(0, 10.0);
                // m_sliderORemovD->SetValue(m_uiState.m_oRemovD);
                // m_sliderORemovD->SetOnValueChanged([this](const double v) {
                //      m_uiState.m_oRemovD = v;
                //     // m_cbVoxelDown.call(&m_uiState);
                //     m_pScene->ForceRedraw();
                // });

                auto *pG = new VGrid(2, v_spacing);
                pG->AddChild(make_shared<Label>("PointSize"));
                pG->AddChild(GiveOwnership(sliderPointSize));
                pG->AddChild(make_shared<Label>("VoxelSize"));
                pG->AddChild(GiveOwnership(m_sliderVsize));
                // pG->AddChild(make_shared<Label>("O-remove N"));
                // pG->AddChild(GiveOwnership(m_sliderORemovN));
                // pG->AddChild(make_shared<Label>("O-remove D"));
                // pG->AddChild(GiveOwnership(m_sliderORemovD));
                panelView->AddChild(GiveOwnership(pG));

                m_btnHiddenRemove = new Button(" Z-Cull ");
                m_btnHiddenRemove->SetOnClicked([this]() {
                    m_uiState.m_vCamPos = m_pScene->GetScene()->GetCamera()->GetPosition();
                    m_cbBtnHiddenRemove.call(&m_uiState);
                    m_pScene->ForceRedraw();
                });

                m_btnFilterReset = new Button(" Reset ");
                m_btnFilterReset->SetOnClicked([this]() {
                    m_cbBtnResetPC.call();
                    m_pScene->ForceRedraw();
                });

                pG = new VGrid(2, v_spacing);
                pG->AddChild(GiveOwnership(m_btnHiddenRemove));
                pG->AddChild(GiveOwnership(m_btnFilterReset));
                panelView->AddChild(GiveOwnership(pG));

                // Scan
                auto panelScan = new CollapsableVert("SCAN", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelScan));

                m_btnScanStart = new Button("        Start        ");
                m_btnScanStart->SetToggleable(true);
                m_btnScanStart->SetOnClicked([this]() {
                    RemoveAllVertexSet();
                    UpdateArea();
                    m_bScanning = !m_bScanning;
                    m_cbBtnScan.call(&m_bScanning);
                    UpdateBtnState();
                    m_pWindow->PostRedraw();
                });
                pH = new Horiz(v_spacing);
                pH->AddChild(GiveOwnership(m_btnScanStart));
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

            void PCscanUI::UpdateTgeometry(const string &name, shared_ptr<t::geometry::PointCloud> sTg)
            {
                for (size_t i = 0; i < m_vObject.size(); i++)
                {
                    DrawObject *pO = &m_vObject[i];
                    if (pO->m_name != name)
                        continue;

                    m_vObject[i].m_sTgeometry = sTg;
                    break;
                }
            }

            float PCscanUI::ConvertToScaledPixels(int px)
            {
                return round(px * m_pWindow->GetScaling());
            }

            void PCscanUI::UpdateSelectableGeometry(void)
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

            void PCscanUI::NewVertexSet(void)
            {
                m_sVertex->NewSet();
                UpdateVertexSetList();
                SelectVertexSet(m_sVertex->GetNumberOfSets() - 1);
            }

            void PCscanUI::SelectVertexSet(int i)
            {
                m_sVertex->SelectSet(i);
                m_listVertexSet->SetSelectedIndex(i);
            }

            void PCscanUI::UpdateVertexSetList(void)
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

                m_pWindow->PostRedraw();
            }

            void PCscanUI::RemoveVertexSet(int i)
            {
                if (i < 0)
                    return;
                m_sVertex->RemoveSet(i);
                UpdateVertexSetList();
            }

            void PCscanUI::RemoveAllVertexSet(void)
            {
                while (m_sVertex->GetNumberOfSets() > 0)
                    m_sVertex->RemoveSet(0);

                UpdateVertexSetList();
            }

            void PCscanUI::UpdateArea(void)
            {
                int iS = m_listVertexSet->GetSelectedIndex();
                size_t nSet = m_sVertex->GetNumberOfSets();

                //no vertex set is selecrted
                if (iS < 0 || iS >= nSet)
                {
                    RemoveGeometry(m_areaName);
                    RemoveDistLabel();
                    return;
                }

                //draw polygon
                std::map<string, set<O3DVisualizerSelections::SelectedIndex>> msSI;
                msSI = m_sVertex->GetSets().at(iS);
                set<O3DVisualizerSelections::SelectedIndex> sSI = msSI[m_modelName];
                int nP = sSI.size();

                //only one point is slected
                if (nP < 2)
                {
                    RemoveGeometry(m_areaName);
                    RemoveDistLabel();
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

                RemoveGeometry(m_areaName);
                AddGeometry(m_areaName, spLS);

                //Distance labels
                RemoveDistLabel();
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

                m_pWindow->PostRedraw();
            }

            void PCscanUI::RemoveDistLabel(void)
            {
                for (shared_ptr<Label3D> spLabel : m_vspDistLabel)
                {
                    m_pScene->RemoveLabel(spLabel);
                }
                m_vspDistLabel.clear();
            }

            double PCscanUI::Area(vector<Vector3d> &vP)
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

            void PCscanUI::OnSaveRGB(void)
            {
                auto dlg = make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::SAVE, "Save File", m_pWindow->GetTheme());
                dlg->AddFilter(".png", "PNG images (.png)");
                dlg->AddFilter("", "All files");
                dlg->SetOnCancel([this]() { this->m_pWindow->CloseDialog(); });
                dlg->SetOnDone([this](const char *path) {
                    this->m_pWindow->CloseDialog();
                    this->ExportCurrentImage(path);
                });
                m_pWindow->ShowDialog(dlg);
            }

            void PCscanUI::OnSavePLY(void)
            {
                auto dlg = make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::SAVE, "Save File", m_pWindow->GetTheme());
                dlg->AddFilter(".ply", "Point Cloud Files (.ply)");
                dlg->AddFilter("", "All files");
                dlg->SetOnCancel([this]() { this->m_pWindow->CloseDialog(); });
                dlg->SetOnDone([this](const char *path) {
                    this->m_pWindow->CloseDialog();

                    ShowMsg("Save", "Saving .Ply file");
                    io::WritePointCloudOption par;
                    par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
                    par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

                    shared_ptr<t::geometry::PointCloud> spTpc = GetGeometry(m_modelName).m_sTgeometry;
                    io::WritePointCloudToPLY(path, spTpc->ToLegacyPointCloud(), par);
                    CloseMsg();
                });
                m_pWindow->ShowDialog(dlg);
            }

            void PCscanUI::OnOpenPLY(void)
            {
                auto dlg = make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::OPEN, "Save File", m_pWindow->GetTheme());
                dlg->AddFilter(".ply", "Point Cloud Files (.ply)");
                dlg->AddFilter("", "All files");
                dlg->SetOnCancel([this]() { this->m_pWindow->CloseDialog(); });
                dlg->SetOnDone([this](const char *path) {
                    this->m_pWindow->CloseDialog();
                    this->m_cbBtnOpenPC.call((void *)path);
                });
                m_pWindow->ShowDialog(dlg);
            }

            void PCscanUI::ShowMsg(const char *pTitle, const char *pMsg, bool bOK)
            {
                auto em = GetTheme().font_size;
                auto margins = Margins(GetTheme().default_margin);
                auto dlg = std::make_shared<Dialog>(pTitle);
                auto layout = std::make_shared<Vert>(em, margins);
                layout->AddChild(std::make_shared<Label>(pMsg));
                if (bOK)
                {
                    auto ok = std::make_shared<Button>("OK");
                    ok->SetOnClicked([this]() { this->CloseDialog(); });
                    layout->AddChild(Horiz::MakeCentered(ok));
                }

                dlg->AddChild(layout);
                ShowDialog(dlg);
            }

            void PCscanUI::CloseMsg(void)
            {
                m_pWindow->CloseDialog();
                m_pWindow->PostRedraw();
            }

        } // namespace visualizer
    }     // namespace visualization
} // namespace open3d

#endif