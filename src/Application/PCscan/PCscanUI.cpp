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

                template <typename T>
                shared_ptr<T> GiveOwnership(T *ptr)
                {
                    return shared_ptr<T>(ptr);
                }
            }

            struct UIState
            {
                Shader m_sceneShader = Shader::STANDARD;
                bool m_bShowSettings = true;
                bool m_bShowAxes = true;

                Eigen::Vector4f bg_color = {0.0f, 0.0f, 0.0f, 0.0f};
                int m_pointSize = 2;
                int m_lineWidth = 5;
                SceneWidget::Controls m_mouseMode = SceneWidget::Controls::FLY;
            };

            struct PCscanUI::Impl
            {
                vector<DrawObject> m_vObject;
                shared_ptr<O3DVisualizerSelections> m_sVertex;

                UIState m_uiState;
                Window *m_pWindow = nullptr;
                SceneWidget *m_pScene = nullptr;
                string m_modelName;
                string m_areaName;
                geometry::LineSet m_areaLine;
                shared_ptr<geometry::LineSet> m_spAreaLine;
                Vector3d m_areaLineCol;

                //UI components
                Vert *m_panelCtrl;

                CollapsableVert *m_panelCam;
                Button *m_btnResetCam;

                CollapsableVert *m_panelScan;
                Button *m_btnScanStart;
                ProgressBar *m_progScan;

                CollapsableVert *m_panelVertexSet;
                Button *m_btnNewVertexSet;
                Button *m_btnDeleteVertexSet;
                ListView *m_listVertexSet;
                Label *m_lArea;

                CollapsableVert *m_panelSetting;
                Slider *m_sliderPointSize;
                //UI components

                void Construct(PCscanUI *w)
                {
                    if (m_pWindow)
                        return;

                    m_modelName = "PCMODEL";
                    m_areaName = "PCAREA";
                    m_areaLineCol = Vector3d(1.0, 0.0, 1.0);
                    m_spAreaLine = make_shared<geometry::LineSet>(m_areaLine);

                    m_pWindow = w;
                    m_pScene = new SceneWidget();
                    m_sVertex = make_shared<O3DVisualizerSelections>(*m_pScene);
                    m_pScene->SetScene(make_shared<Open3DScene>(w->GetRenderer()));
                    m_pScene->EnableSceneCaching(true); // smoother UI with large geometry
                    m_pScene->SetOnPointsPicked(
                        [this](const map<
                                   string,
                                   vector<pair<size_t, Eigen::Vector3d>>>
                                   &indices,
                               int keymods) {
                            if (keymods & int(KeyModifier::SHIFT))
                                m_sVertex->UnselectIndices(indices);
                            else
                                m_sVertex->SelectIndices(indices);

                            UpdateArea();
                        });
                    w->AddChild(GiveOwnership(m_pScene));

                    auto o3dscene = m_pScene->GetScene();
                    o3dscene->SetBackground(m_uiState.bg_color);
                    Eigen::Vector3f sun_dir = {0, 0, 0};
                    o3dscene->SetLighting(Open3DScene::LightingProfile::NO_SHADOWS, sun_dir);

                    MakeCtrlPanel();
                    ShowAxes(true);
                    SetPointSize(m_uiState.m_pointSize); // sync selections_' point size
                    SetLineWidth(m_uiState.m_lineWidth);
                    SetMouseCameraMode(m_uiState.m_mouseMode);
                    ResetCameraToDefault();
                }

                void MakeCtrlPanel(void)
                {
                    auto em = m_pWindow->GetTheme().font_size;
                    auto half_em = int(round(0.5f * float(em)));
                    auto v_spacing = int(round(0.25 * float(em)));

                    m_panelCtrl = new Vert(half_em);
                    m_pWindow->AddChild(GiveOwnership(m_panelCtrl));

                    Margins margins(em, 0, half_em, 0);
                    Margins tabbed_margins(0, half_em, 0, 0);

                    // Camera
                    m_panelCam = new CollapsableVert("CAMERA", v_spacing, margins);
                    m_panelCtrl->AddChild(GiveOwnership(m_panelCam));

                    m_btnResetCam = new Button("Reset Camera  ");
                    m_btnResetCam->SetOnClicked([this]() {
                        this->ResetCameraToDefault();
                    });
                    auto h = new Horiz(v_spacing);
                    h->AddChild(GiveOwnership(m_btnResetCam));
                    h->AddStretch();
                    m_panelCam->AddChild(GiveOwnership(h));

                    // Scan
                    m_panelScan = new CollapsableVert("SCAN", v_spacing, margins);
                    m_panelCtrl->AddChild(GiveOwnership(m_panelScan));

                    m_btnScanStart = new Button("Start New Scan");
                    m_btnScanStart->SetToggleable(true);
                    m_btnScanStart->SetOnClicked([this]() {
                        //toggle
                        SetMouseCameraMode(m_uiState.m_mouseMode);
                    });
                    h = new Horiz(v_spacing);
                    h->AddChild(GiveOwnership(m_btnScanStart));
                    h->AddStretch();
                    m_panelScan->AddChild(GiveOwnership(h));

                    const char *strMem = "Used memory:";
                    h = new Horiz();
                    h->AddChild(make_shared<Label>(strMem));
                    h->AddStretch();
                    m_panelScan->AddChild(GiveOwnership(h));

                    m_progScan = new ProgressBar();
                    m_progScan->SetValue(0.5);
                    h = new Horiz(v_spacing);
                    h->AddChild(GiveOwnership(m_progScan));
                    m_panelScan->AddChild(GiveOwnership(h));

                    // Vertex selection
                    m_panelVertexSet = new CollapsableVert("MEASURE", v_spacing, margins);
                    m_panelCtrl->AddChild(GiveOwnership(m_panelVertexSet));

                    m_btnNewVertexSet = new SmallButton(" + ");
                    m_btnNewVertexSet->SetOnClicked(
                        [this]() {
                            NewVertexSet();
                            SetMousePickingMode();
                        });
                    m_btnDeleteVertexSet = new SmallButton(" - ");
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
                    m_panelVertexSet->AddChild(GiveOwnership(h));

                    const char *selection_help = "(Ctrl-click to select a point)";
                    h = new Horiz();
                    h->AddChild(make_shared<Label>(selection_help));
                    h->AddStretch();
                    m_panelVertexSet->AddChild(GiveOwnership(h));

                    m_listVertexSet = new ListView();
                    m_listVertexSet->SetOnValueChanged([this](const char *, bool) {
                        SelectVertexSet(m_listVertexSet->GetSelectedIndex());
                        UpdateArea();
                    });
                    m_panelVertexSet->AddChild(GiveOwnership(m_listVertexSet));

                    // Area Calculation
                    m_lArea = new Label();
                    m_lArea->SetBackgroundColor(Color(1.0, 0, 0));
                    h = new Horiz();
                    h->AddChild(GiveOwnership(m_lArea));
                    h->AddStretch();
                    m_panelVertexSet->AddChild(GiveOwnership(h));

                    // Scene controls
                    m_panelSetting = new CollapsableVert("SETTING", v_spacing, margins);
                    m_panelSetting->SetIsOpen(false);
                    m_panelCtrl->AddChild(GiveOwnership(m_panelSetting));
                    m_sliderPointSize = new Slider(Slider::INT);
                    m_sliderPointSize->SetLimits(1, 10);
                    m_sliderPointSize->SetValue(m_uiState.m_pointSize);
                    m_sliderPointSize->SetOnValueChanged([this](const double newValue) {
                        this->SetPointSize(int(newValue));
                    });

                    auto *grid = new VGrid(2, v_spacing);
                    m_panelSetting->AddChild(GiveOwnership(grid));

                    grid->AddChild(make_shared<Label>("PointSize"));
                    grid->AddChild(GiveOwnership(m_sliderPointSize));
                }

                void AddGeometry(const string &name,
                                 shared_ptr<geometry::Geometry3D> geom,
                                 rendering::Material *material = nullptr,
                                 bool bVisible = true)
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

                        auto cloud = dynamic_pointer_cast<geometry::PointCloud>(geom);
                        auto lines = dynamic_pointer_cast<geometry::LineSet>(geom);
                        auto mesh = dynamic_pointer_cast<geometry::MeshBase>(geom);

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

                        mat.base_color = CalcDefaultUnlitColor();
                        mat.shader = kShaderUnlit;
                        if (lines)
                        {
                            mat.shader = kShaderUnlitLines;
                            mat.line_width = m_uiState.m_lineWidth * m_pWindow->GetScaling();
                        }
                        is_default_color = true;
                        if (has_colors)
                        {
                            mat.base_color = {1.0f, 1.0f, 1.0f, 1.0f};
                            is_default_color = false;
                        }
                        if (has_normals)
                        {
                            mat.base_color = {1.0f, 1.0f, 1.0f, 1.0f};
                            mat.shader = kShaderLit;
                            is_default_color = false;
                        }
                        mat.point_size = ConvertToScaledPixels(m_uiState.m_pointSize);
                    }

                    m_vObject.push_back({name, geom, nullptr, mat, bVisible});

                    auto scene = m_pScene->GetScene();
                    scene->AddGeometry(name, geom.get(), mat);
                    scene->GetScene()->GeometryShadows(name, false, false);
                    scene->ShowGeometry(name, bVisible);

                    SetPointSize(m_uiState.m_pointSize);
                    m_pScene->ForceRedraw();
                }

                void AddTgeometry(const string &name,
                                  shared_ptr<t::geometry::PointCloud> tgeom,
                                  bool bVisible = true)
                {
                    bool no_shadows = false;
                    Material mat;
                    bool has_colors = false;
                    bool has_normals = false;

                    auto t_cloud = dynamic_pointer_cast<t::geometry::PointCloud>(tgeom);
                    auto t_mesh = dynamic_pointer_cast<t::geometry::TriangleMesh>(tgeom);
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

                    mat.base_color = CalcDefaultUnlitColor();
                    mat.shader = kShaderUnlit;

                    if (has_colors)
                        mat.base_color = {1.0f, 1.0f, 1.0f, 1.0f};

                    if (has_normals)
                    {
                        mat.base_color = {1.0f, 1.0f, 1.0f, 1.0f};
                        mat.shader = kShaderLit;
                    }

                    mat.point_size = ConvertToScaledPixels(m_uiState.m_pointSize);

                    m_vObject.push_back({name, nullptr, tgeom, mat, bVisible});
                    auto scene = m_pScene->GetScene();

                    scene->AddGeometry(name, tgeom.get(), mat);
                    scene->GetScene()->GeometryShadows(name, false, false);
                    scene->ShowGeometry(name, bVisible);

                    SetPointSize(m_uiState.m_pointSize);
                    m_pScene->ForceRedraw();
                }

                void RemoveGeometry(const string &name)
                {
                    for (size_t i = 0; i < m_vObject.size(); ++i)
                    {
                        if (m_vObject[i].m_name == name)
                        {
                            //                            m_vObject.erase(m_vObject.begin() + i);
                            break;
                        }
                    }

                    m_pScene->GetScene()->RemoveGeometry(name);

                    // Bounds have changed, so update the selection point size, since they
                    // depend on the bounds.
                    SetPointSize(m_uiState.m_pointSize);

                    m_pScene->ForceRedraw();
                }

                DrawObject GetGeometry(const string &name) const
                {
                    for (auto &o : m_vObject)
                    {
                        if (o.m_name == name)
                            return o;
                    }
                    return DrawObject();
                }

                void SetupCamera(float fov,
                                 const Eigen::Vector3f &center,
                                 const Eigen::Vector3f &eye,
                                 const Eigen::Vector3f &up)
                {
                    auto scene = m_pScene->GetScene();
                    m_pScene->SetupCamera(fov, scene->GetBoundingBox(), {0.0f, 0.0f, 0.0f});
                    scene->GetCamera()->LookAt(center, eye, up);
                    m_pScene->ForceRedraw();
                }

                void ResetCameraToDefault(void)
                {
                    auto scene = m_pScene->GetScene();
                    m_pScene->SetupCamera(60.0f, scene->GetBoundingBox(), {0.0f, 0.0f, 0.0f});

                    Eigen::Vector3f vCenter(0, 0, 0);
                    Eigen::Vector3f vEye(0, 0, 1);
                    Eigen::Vector3f vUp(1, 0, 0);
                    scene->GetCamera()->LookAt(vCenter, vEye, vUp);

                    m_pScene->ForceRedraw();
                }

                void SetBackground(const Eigen::Vector4f &bg_color,
                                   shared_ptr<geometry::Image> bg_image)
                {
                    m_uiState.bg_color = bg_color;
                    auto scene = m_pScene->GetScene();
                    scene->SetBackground(m_uiState.bg_color, bg_image);
                    m_pScene->ForceRedraw();
                }

                void ShowSettings(bool show)
                {
                    m_uiState.m_bShowSettings = show;
                    m_panelCtrl->SetVisible(show);
                    m_pWindow->SetNeedsLayout();
                }

                void ShowAxes(bool show)
                {
                    m_uiState.m_bShowAxes = show;
                    m_pScene->GetScene()->ShowAxes(show);
                    m_pScene->ForceRedraw();
                }

                void SetPointSize(int px)
                {
                    m_uiState.m_pointSize = px;
                    m_sliderPointSize->SetValue(double(px));

                    px = int(ConvertToScaledPixels(px));
                    for (auto &o : m_vObject)
                    {
                        o.m_material.point_size = float(px);
                        OverrideMaterial(o.m_name, o.m_material, m_uiState.m_sceneShader);
                    }
                    auto bbox = m_pScene->GetScene()->GetBoundingBox();
                    auto xdim = bbox.max_bound_.x() - bbox.min_bound_.x();
                    auto ydim = bbox.max_bound_.y() - bbox.min_bound_.z();
                    auto zdim = bbox.max_bound_.z() - bbox.min_bound_.y();
                    auto psize = double(max(5, px)) * 0.000666 * max(xdim, max(ydim, zdim));
                    m_sVertex->SetPointSize(psize);

                    m_pScene->SetPickablePointSize(px);
                    m_pScene->ForceRedraw();
                }

                void SetLineWidth(int px)
                {
                    m_uiState.m_lineWidth = px;

                    px = int(ConvertToScaledPixels(px));
                    for (auto &o : m_vObject)
                    {
                        o.m_material.line_width = float(px);
                        OverrideMaterial(o.m_name, o.m_material, m_uiState.m_sceneShader);
                    }
                    m_pScene->ForceRedraw();
                }

                void SetShader(Shader shader)
                {
                    m_uiState.m_sceneShader = shader;
                    for (auto &o : m_vObject)
                    {
                        OverrideMaterial(o.m_name, o.m_material, shader);
                    }
                    m_pScene->ForceRedraw();
                }

                void OverrideMaterial(const string &name,
                                      const Material &original_material,
                                      Shader shader)
                {
                    bool is_lines = (original_material.shader == "unlitLine" ||
                                     original_material.shader == "lines");
                    auto scene = m_pScene->GetScene();
                    // Lines are already unlit, so keep using the original shader when in
                    // unlit mode so that we can keep the wide lines.
                    if (shader == Shader::STANDARD ||
                        (shader == Shader::UNLIT && is_lines))
                    {
                        scene->GetScene()->OverrideMaterial(name, original_material);
                    }
                }

                float ConvertToScaledPixels(int px)
                {
                    return round(px * m_pWindow->GetScaling());
                }

                void SetMouseCameraMode(SceneWidget::Controls mode)
                {
                    if (m_sVertex->IsActive())
                    {
                        m_sVertex->MakeInactive();
                    }

                    m_pScene->SetViewControls(mode);
                }

                void SetMousePickingMode(void)
                {
                    if (m_sVertex->GetNumberOfSets() == 0)
                    {
                        NewVertexSet();
                    }

                    UpdateSelectableGeometry();
                    m_sVertex->MakeActive();
                }

                void UpdateSelectableGeometry(void)
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

                vector<O3DVisualizerSelections::SelectionSet> GetSelectionSets() const
                {
                    return m_sVertex->GetSets();
                }

                void NewVertexSet(void)
                {
                    m_sVertex->NewSet();
                    UpdateVertexSetList();
                    SelectVertexSet(int(m_sVertex->GetNumberOfSets()) - 1);
                }

                void RemoveVertexSet(int index)
                {
                    m_sVertex->RemoveSet(index);
                    if (m_sVertex->GetNumberOfSets() == 0)
                    {
                        // You can remove the last set, but there must always be one
                        // set, so we re-create one. (So removing the last set has the
                        // effect of clearing it.)
                        m_sVertex->NewSet();
                    }
                    UpdateVertexSetList();
                }

                void SelectVertexSet(int index)
                {
                    m_listVertexSet->SetSelectedIndex(index);
                    m_sVertex->SelectSet(index);
                }

                void UpdateVertexSetList(void)
                {
                    size_t n = m_sVertex->GetNumberOfSets();
                    int idx = m_listVertexSet->GetSelectedIndex();
                    idx = max(0, idx);
                    idx = min(idx, int(n) - 1);

                    vector<string> items;
                    items.reserve(n);
                    for (size_t i = 0; i < n; ++i)
                    {
                        stringstream s;
                        s << "Set " << (i + 1);
                        items.push_back(s.str());
                    }
                    m_listVertexSet->SetItems(items);

                    SelectVertexSet(idx);
                    m_pWindow->PostRedraw();
                }

                void UpdateArea(void)
                {
                    int index = m_listVertexSet->GetSelectedIndex();
                    if (index < 0)
                    {
                        UpdateAreaLabel(-1.0);
                        return;
                    }

                    //draw rectangle
                    map<string, set<O3DVisualizerSelections::SelectedIndex>> msSI;
                    msSI = m_sVertex->GetSets().at(index);
                    set<O3DVisualizerSelections::SelectedIndex> sSI = msSI[m_modelName];
                    int nP = sSI.size();
                    if (nP < 3)
                    {
                        UpdateAreaLabel(-1.0);
                        return;
                    }

                    m_areaLine.points_.clear();
                    m_areaLine.lines_.clear();
                    m_areaLine.colors_.clear();
                    for (int i = 0; i < nP; i++)
                    {
                        for (O3DVisualizerSelections::SelectedIndex sI : sSI)
                        {
                            if(sI.order != i)continue;
                            m_areaLine.points_.push_back(sI.point);
                            break;
                        }
                    }

                    nP = m_areaLine.points_.size();
                    for (int i = 0; i < nP; i++)
                    {
                        int j = i + 1;
                        if (j >= nP)
                            j = 0;
                        m_areaLine.lines_.push_back(Vector2i(i, j));
                        m_areaLine.colors_.push_back(m_areaLineCol);
                    }

                    RemoveGeometry(m_areaName);
                    AddGeometry(m_areaName, GiveOwnership(&m_areaLine));
                    //                    AddGeometry(m_areaName, m_spAreaLine);

                    //Calc area
                    double S = 0;
                    for (int i = 1; i <= nP - 2; i++)
                    {
                        S +=
                            CalcArea(
                                m_areaLine.points_[0],
                                m_areaLine.points_[i],
                                m_areaLine.points_[i + 1]);
                    }

                    UpdateAreaLabel(S);
                    m_pWindow->PostRedraw();
                }

                void UpdateAreaLabel(double S)
                {
                    string strS = "Set not selected";
                    if (S >= 0.0)
                    {
                        char buf[16];
                        string format = "%.3f";
                        snprintf(buf, 16, format.c_str(), S);
                        strS = "Area = " + string(buf) + " m^2";
                    }

                    m_lArea->SetText(strS.c_str());
                }

                double CalcArea(const Vector3d &p1,
                                const Vector3d &p2,
                                const Vector3d &p3)
                {
                    Vector3d d12 = p2 - p1;
                    Vector3d d23 = p3 - p2;
                    Vector3d d31 = p1 - p3;
                    double a = d12.norm();
                    double b = d23.norm();
                    double c = d31.norm();
                    double s = (a + b + c) / 2;
                    return sqrt(s * (s - a) * (s - b) * (s - c));
                }

                void ExportCurrentImage(const string &path)
                {
                    m_pScene->EnableSceneCaching(false);
                    m_pScene->GetScene()->GetScene()->RenderToImage(
                        [this, path](shared_ptr<geometry::Image> image) mutable {
                            if (!io::WriteImage(path, *image))
                            {
                                this->m_pWindow->ShowMessageBox(
                                    "Error",
                                    (string("Could not write image to ") +
                                     path + ".")
                                        .c_str());
                            }
                            m_pScene->EnableSceneCaching(true);
                        });
                }

                void OnExportRGB()
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

                Eigen::Vector4f CalcDefaultUnlitColor()
                {
                    float luminosity = 0.21f * m_uiState.bg_color.x() +
                                       0.72f * m_uiState.bg_color.y() +
                                       0.07f * m_uiState.bg_color.z();
                    if (luminosity >= 0.5f)
                    {
                        return {0.0f, 0.0f, 0.0f, 1.0f};
                    }
                    else
                    {
                        return {1.0f, 1.0f, 1.0f, 1.0f};
                    }
                }
            };

            // ----------------------------------------------------------------------------
            PCscanUI::PCscanUI(const string &title, int width, int height)
                : Window(title, width, height), impl_(new PCscanUI::Impl())
            {
                impl_->Construct(this);
                Application::GetInstance().SetMenubar(NULL);
            }

            PCscanUI::~PCscanUI() {}

            Open3DScene *PCscanUI::GetScene() const
            {
                return impl_->m_pScene->GetScene().get();
            }

            void PCscanUI::SetBackground(
                const Eigen::Vector4f &bg_color,
                shared_ptr<geometry::Image> bg_image /*= nullptr*/)
            {
                impl_->SetBackground(bg_color, bg_image);
            }

            void PCscanUI::AddGeometry(const string &name,
                                       shared_ptr<geometry::Geometry3D> geom,
                                       rendering::Material *material /*= nullptr*/,
                                       bool bVisible /*= true*/)
            {
                impl_->AddGeometry(name, geom, material, bVisible);
            }

            void PCscanUI::AddPointCloud(const string &name,
                                         shared_ptr<geometry::PointCloud> sPC,
                                         rendering::Material *material /*= nullptr*/,
                                         bool bVisible /*= true*/)
            {
                t::geometry::PointCloud tpcd = open3d::t::geometry::PointCloud::
                    FromLegacyPointCloud(
                        *sPC.get(),
                        core::Dtype::Float32);

                shared_ptr<t::geometry::PointCloud> spP =
                    make_shared<t::geometry::PointCloud>(tpcd);

                impl_->AddTgeometry(name, spP, bVisible);
            }

            void PCscanUI::UpdatePointCloud(const string &name,
                                            shared_ptr<geometry::PointCloud> sPC)
            {
                gui::Application::GetInstance().PostToMainThread(
                    this, [this, sPC, name]() {
                        t::geometry::PointCloud tpcd = open3d::t::geometry::PointCloud::
                            FromLegacyPointCloud(
                                *sPC.get(),
                                core::Dtype::Float32);

                        impl_->m_pScene->GetScene()->GetScene()->UpdateGeometry(
                            name,
                            tpcd,
                            open3d::visualization::rendering::Scene::kUpdatePointsFlag |
                                open3d::visualization::rendering::Scene::kUpdateColorsFlag);
                        impl_->m_pScene->ForceRedraw();
                    });
            }

            void PCscanUI::RemoveGeometry(const string &name)
            {
                return impl_->RemoveGeometry(name);
            }

            DrawObject PCscanUI::GetGeometry(const string &name) const
            {
                return impl_->GetGeometry(name);
            }

            void PCscanUI::ShowSettings(bool show) { impl_->ShowSettings(show); }

            void PCscanUI::ShowAxes(bool show) { impl_->ShowAxes(show); }

            void PCscanUI::SetPointSize(int point_size)
            {
                impl_->SetPointSize(point_size);
            }

            void PCscanUI::SetLineWidth(int line_width)
            {
                impl_->SetLineWidth(line_width);
            }

            vector<O3DVisualizerSelections::SelectionSet>
            PCscanUI::GetSelectionSets() const
            {
                return impl_->GetSelectionSets();
            }

            void PCscanUI::SetupCamera(float fov,
                                       const Eigen::Vector3f &center,
                                       const Eigen::Vector3f &eye,
                                       const Eigen::Vector3f &up) {}

            void PCscanUI::ResetCameraToDefault()
            {
                return impl_->ResetCameraToDefault();
            }

            void PCscanUI::ExportCurrentImage(const string &path)
            {
                impl_->ExportCurrentImage(path);
            }

            void PCscanUI::Layout(const Theme &theme)
            {
                auto em = theme.font_size;
                int settings_width = 15 * theme.font_size;

                auto f = GetContentRect();
                if (impl_->m_panelCtrl->IsVisible())
                {
                    impl_->m_pScene->SetFrame(Rect(f.x, f.y, f.width - settings_width, f.height));
                    impl_->m_panelCtrl->SetFrame(Rect(f.GetRight() - settings_width, f.y, settings_width, f.height));
                }
                else
                {
                    impl_->m_pScene->SetFrame(f);
                }

                Super::Layout(theme);
            }

        } // namespace visualizer
    }     // namespace visualization
} // namespace open3d

#endif