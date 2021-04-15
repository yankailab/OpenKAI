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
                static const string kShaderLit = "defaultLit";
                static const string kShaderUnlit = "defaultUnlit";
                static const string kShaderUnlitLines = "unlitLine";
                static const string kShaderDepth = "depth";

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
                m_modelName = "PCMODEL";
                m_areaName = "PCAREA";

                m_pWindow = this;
                m_pScene = new SceneWidget();
                m_sVertex = make_shared<O3DVisualizerSelections>(*m_pScene);
                m_pScene->SetScene(make_shared<Open3DScene>(m_pWindow->GetRenderer()));
                m_pScene->SetOnPointsPicked(
                    [this](const map<
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

                InitCtrlPanel();
                UpdateUIstate();
                SetMouseCameraMode();
            }

            void PCscanUI::AddGeometry(const string &name,
                                       shared_ptr<geometry::Geometry3D> sTg,
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

                    auto cloud = dynamic_pointer_cast<geometry::PointCloud>(sTg);
                    auto lines = dynamic_pointer_cast<geometry::LineSet>(sTg);
                    auto mesh = dynamic_pointer_cast<geometry::MeshBase>(sTg);

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
                    mat.shader = kShaderUnlit;
                    if (lines)
                    {
                        mat.shader = kShaderUnlitLines;
                        mat.line_width = m_uiState.m_lineWidth * m_pWindow->GetScaling();
                    }
                    is_default_color = true;
                    if (has_colors)
                    {
                        is_default_color = false;
                    }
                    if (has_normals)
                    {
                        mat.shader = kShaderLit;
                        is_default_color = false;
                    }
                    mat.point_size = ConvertToScaledPixels(m_uiState.m_pointSize);
                }

                m_vObject.push_back({name, sTg, nullptr, mat, bVisible});

                auto scene = m_pScene->GetScene();
                scene->AddGeometry(name, sTg.get(), mat);
                scene->GetScene()->GeometryShadows(name, false, false);
                scene->ShowGeometry(name, bVisible);

                SetPointSize(m_uiState.m_pointSize);
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
                mat.shader = kShaderUnlit;
                mat.point_size = ConvertToScaledPixels(m_uiState.m_pointSize);

                m_vObject.push_back({name, nullptr, sTg, mat, bVisible});
                auto scene = m_pScene->GetScene();

                scene->AddGeometry(name, sTg.get(), mat, false);
                scene->GetScene()->GeometryShadows(name, false, false);
                scene->ShowGeometry(name, bVisible);

                SetPointSize(m_uiState.m_pointSize);
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
                SetPointSize(m_uiState.m_pointSize);

                m_pScene->ForceRedraw();
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

                SetPointSize(m_uiState.m_pointSize);
                SetLineWidth(m_uiState.m_lineWidth);

                m_pWindow->SetNeedsLayout();
                m_pScene->ForceRedraw();
            }

            void PCscanUI::SetPointSize(int px)
            {
                m_uiState.m_pointSize = px;

                px = int(ConvertToScaledPixels(px));
                for (auto &o : m_vObject)
                {
                    o.m_material.point_size = float(px);
                    m_pScene->GetScene()->GetScene()->OverrideMaterial(o.m_name, o.m_material);
                }

                if (m_sVertex->GetNumberOfSets())
                    m_sVertex->SetPointSize(m_uiState.m_selectPointSize);

                m_pScene->SetPickablePointSize(px);
                m_pScene->ForceRedraw();
            }

            void PCscanUI::SetLineWidth(int px)
            {
                m_uiState.m_lineWidth = px;

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
            }

            void PCscanUI::SetMousePickingMode(void)
            {
                UpdateSelectableGeometry();
                m_sVertex->MakeActive();
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

            void PCscanUI::SetCbBtnSavePC(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbBtnSavePC.add(pCb, pPCV);
            }

            void PCscanUI::SetCbBtnCamSet(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbBtnCamSet.add(pCb, pPCV);
            }

            void PCscanUI::SetCbBtnHiddenRemove(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbBtnHiddenRemove.add(pCb, pPCV);
            }

            void PCscanUI::SetCbBtnFilterReset(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbBtnFilterReset.add(pCb, pPCV);
            }

            void PCscanUI::SetCbFilter(OnBtnClickedCb pCb, void *pPCV)
            {
                m_cbFilter.add(pCb, pPCV);
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

            vector<O3DVisualizerSelections::SelectionSet> PCscanUI::GetSelectionSets() const
            {
                return m_sVertex->GetSets();
            }

            Open3DScene *PCscanUI::GetScene() const
            {
                return m_pScene->GetScene().get();
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
                auto em = theme.font_size;
                int settings_width = m_uiState.m_panelWidth * theme.font_size;

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

                auto btnCamSavePC = new Button(" Save PLY ");
                btnCamSavePC->SetOnClicked([this]() {
                    OnSavePLY();
                });

                auto btnCamSaveRGB = new Button(" Save PNG ");
                btnCamSaveRGB->SetOnClicked([this]() {
                    OnSaveRGB();
                });

                auto h = new Horiz(v_spacing);
                h->AddChild(GiveOwnership(btnCamSavePC));
                h->AddChild(GiveOwnership(btnCamSaveRGB));
                h->AddStretch();
                panelFile->AddChild(GiveOwnership(h));
                panelFile->SetIsOpen(false);

                // Camera
                auto panelCam = new CollapsableVert("CAMERA", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelCam));

                auto btnCamAuto = new Button(" Auto ");
                btnCamAuto->SetOnClicked([this]() {
                    bool b = true;
                    m_cbBtnCamSet.call(&b);
                    m_pScene->ForceRedraw();
                    SetMouseCameraMode();
                });

                auto btnCamReset = new Button(" Reset ");
                btnCamReset->SetOnClicked([this]() {
                    bool b = false;
                    m_cbBtnCamSet.call(&b);
                    m_pScene->ForceRedraw();
                    SetMouseCameraMode();
                });

                h = new Horiz(v_spacing);
                h->AddChild(GiveOwnership(btnCamAuto));
                h->AddChild(GiveOwnership(btnCamReset));
                h->AddStretch();
                panelCam->AddChild(GiveOwnership(h));

                // View
                auto panelView = new CollapsableVert("VIEW", v_spacing, margins);
                panelView->SetIsOpen(true);
                m_panelCtrl->AddChild(GiveOwnership(panelView));

                auto sliderPointSize = new Slider(Slider::INT);
                sliderPointSize->SetLimits(1, 10);
                sliderPointSize->SetValue(m_uiState.m_pointSize);
                sliderPointSize->SetOnValueChanged([this](const double v)
                {
                    SetPointSize(int(v));
                });

                m_sliderVsize = new Slider(Slider::DOUBLE);
                m_sliderVsize->SetLimits(0.001, 1.0);
                m_sliderVsize->SetValue(m_uiState.m_voxelSize);
                m_sliderVsize->SetOnValueChanged([this](const double v)
                {
                    m_uiState.m_voxelSize = v;
                    m_cbFilter.call(&m_uiState);
                    m_pScene->ForceRedraw();
                });

                auto *grid = new VGrid(2, v_spacing);
                grid->AddChild(make_shared<Label>("PointSize"));
                grid->AddChild(GiveOwnership(sliderPointSize));
                grid->AddChild(make_shared<Label>("VoxelSize"));
                grid->AddChild(GiveOwnership(m_sliderVsize));
                panelView->AddChild(GiveOwnership(grid));

                m_btnHiddenRemove = new Button(" Z-Culling ");
                m_btnHiddenRemove->SetOnClicked([this]()
                {
                    m_uiState.m_vCamPos = m_pScene->GetScene()->GetCamera()->GetPosition();
                    m_cbBtnHiddenRemove.call(&m_uiState);
                    m_pScene->ForceRedraw();
                });

                m_btnFilterReset = new Button(" Reset ");
                m_btnFilterReset->SetOnClicked([this]()
                {
                    m_cbBtnFilterReset.call();
                    m_pScene->ForceRedraw();
                });

                h = new Horiz(v_spacing);
                h->AddChild(GiveOwnership(m_btnHiddenRemove));
                h->AddStretch();
                h->AddChild(GiveOwnership(m_btnFilterReset));
                h->AddStretch();
                panelView->AddChild(GiveOwnership(h));

                // Scan
                auto panelScan = new CollapsableVert("SCAN", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelScan));

                m_btnScanStart = new Button("        Start        ");
                m_btnScanStart->SetToggleable(true);
                m_btnScanStart->SetOnClicked([this]()
                {
                    SetMouseCameraMode();
                    RemoveAllVertexSet();

                    m_bScanning = !m_bScanning;
                    m_btnScanStart->SetOn(m_bScanning);
                    if (m_bScanning)
                    {
                        m_btnScanStart->SetText("        Stop        ");
                        m_btnNewVertexSet->SetEnabled(false);
                        m_btnDeleteVertexSet->SetEnabled(false);
                        m_listVertexSet->SetEnabled(false);
                        m_sliderVsize->SetEnabled(false);
                        m_btnHiddenRemove->SetEnabled(false);
                        m_btnFilterReset->SetEnabled(false);
                        m_labelArea->SetText("Area not selected");
                    }
                    else
                    {
                        m_btnScanStart->SetText("        Start        ");
                        m_btnNewVertexSet->SetEnabled(true);
                        m_btnDeleteVertexSet->SetEnabled(true);
                        m_listVertexSet->SetEnabled(true);
                        m_sliderVsize->SetEnabled(true);
                        m_btnHiddenRemove->SetEnabled(true);
                        m_btnFilterReset->SetEnabled(true);
                    }

                    m_cbBtnScan.call(&m_bScanning);
                    m_pWindow->PostRedraw();
                });
                h = new Horiz(v_spacing);
                h->AddChild(GiveOwnership(m_btnScanStart));
                h->AddStretch();
                panelScan->AddChild(GiveOwnership(h));

                m_labelProg = new Label("Memory used: 0%");
                h = new Horiz(v_spacing);
                h->AddChild(GiveOwnership(m_labelProg));
                h->AddStretch();
                panelScan->AddChild(GiveOwnership(h));

                m_progScan = new ProgressBar();
                m_progScan->SetValue(0.0);
                h = new Horiz(v_spacing);
                h->AddChild(GiveOwnership(m_progScan));
                panelScan->AddChild(GiveOwnership(h));

                // Vertex selection
                auto panelVertexSet = new CollapsableVert("MEASURE", v_spacing, margins);
                m_panelCtrl->AddChild(GiveOwnership(panelVertexSet));

                m_btnNewVertexSet = new SmallButton("  +  ");
                m_btnNewVertexSet->SetOnClicked(
                    [this]() {
                        NewVertexSet();
                        SetMousePickingMode();
                    });
                m_btnDeleteVertexSet = new SmallButton("  -  ");
                m_btnDeleteVertexSet->SetOnClicked(
                    [this]() {
                        int idx = m_listVertexSet->GetSelectedIndex();
                        RemoveVertexSet(idx);
                    });
                h = new Horiz(v_spacing);
                h->AddChild(make_shared<Label>("Select Area"));
                h->AddStretch();
                h->AddChild(GiveOwnership(m_btnNewVertexSet));
                h->AddChild(GiveOwnership(m_btnDeleteVertexSet));
                panelVertexSet->AddChild(GiveOwnership(h));

                h = new Horiz(v_spacing);
                m_labelArea = new Label("Area not selected");
                h->AddChild(GiveOwnership(m_labelArea));
                h->AddStretch();
                panelVertexSet->AddChild(GiveOwnership(h));

                m_listVertexSet = new ListView();
                m_listVertexSet->SetOnValueChanged(
                    [this](const char *, bool) {
                        SelectVertexSet(m_listVertexSet->GetSelectedIndex());
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
                pickable.reserve(m_vObject.size());
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
                SelectVertexSet(int(m_sVertex->GetNumberOfSets()) - 1);
                UpdateArea();
            }

            void PCscanUI::SelectVertexSet(int index)
            {
                m_listVertexSet->SetSelectedIndex(index);
                m_sVertex->SelectSet(index);
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

                if (n > 0)
                {
                    int idx = m_listVertexSet->GetSelectedIndex();
                    idx = min(idx, int(n) - 1);
                    idx = max(0, idx);
                    SelectVertexSet(idx);
                }

                m_pWindow->PostRedraw();
            }

            void PCscanUI::RemoveVertexSet(int index)
            {
                if (index < 0)
                    return;
                m_sVertex->RemoveSet(index);
                UpdateVertexSetList();
                UpdateArea();
            }

            void PCscanUI::RemoveAllVertexSet(void)
            {
                while (m_sVertex->GetNumberOfSets() > 0)
                    m_sVertex->RemoveSet(0);

                UpdateVertexSetList();
                UpdateArea();
            }

            void PCscanUI::UpdateArea(void)
            {
                int iS = m_listVertexSet->GetSelectedIndex();
                size_t nSet = m_sVertex->GetNumberOfSets();
                if (iS < 0 || iS >= nSet)
                {
                    RemoveGeometry(m_areaName);
                    RemoveDistLabel();
                    return;
                }

                //draw polygon
                map<string, set<O3DVisualizerSelections::SelectedIndex>> msSI;
                msSI = m_sVertex->GetSets().at(iS);
                set<O3DVisualizerSelections::SelectedIndex> sSI = msSI[m_modelName];
                int nP = sSI.size();
                if (nP < 3)
                {
                    RemoveGeometry(m_areaName);
                    RemoveDistLabel();
                    return;
                }

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
                    this->m_cbBtnSavePC.call((void *)path);
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

            void PCscanUI::ShowMsg(const char* pTitle, const char* pMsg, bool bOK)
            {
                auto em = GetTheme().font_size;
                auto margins = Margins(GetTheme().default_margin);
                auto dlg = std::make_shared<Dialog>(pTitle);
                auto layout = std::make_shared<Vert>(em, margins);
                layout->AddChild(std::make_shared<Label>(pMsg));
                if(bOK)
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