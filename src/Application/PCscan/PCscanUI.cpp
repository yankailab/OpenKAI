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
                static const std::string kShaderLit = "defaultLit";
                static const std::string kShaderUnlit = "defaultUnlit";
                static const std::string kShaderUnlitLines = "unlitLine";

                template <typename T>
                std::shared_ptr<T> GiveOwnership(T *ptr)
                {
                    return std::shared_ptr<T>(ptr);
                }

            } // namespace

            struct PCscanUI::Impl
            {
                std::set<std::string> m_sGname;
                std::vector<DrawObject> m_vObject;
                std::shared_ptr<O3DVisualizerSelections> m_sSelection;
                bool bUpdateSelection = true;

                UIState m_uiState;
                Window *m_pWindow = nullptr;
                SceneWidget *m_pScene = nullptr;

                struct
                {
                    Vert *m_panelAll;
                    CollapsableVert *m_panelScan;

                    CollapsableVert *m_panelVertexSet;
                    Button *m_btnNewVertexSet;
                    Button *m_btnDeleteVertexSet;
                    ListView *m_listSelectSet;

                    CollapsableVert *m_panelSetting;
                    Slider *m_sliderPointSize;
                } settings;

                void Construct(PCscanUI *w)
                {
                    if (m_pWindow)
                        return;

                    m_pWindow = w;
                    m_pScene = new SceneWidget();
                    m_sSelection = std::make_shared<O3DVisualizerSelections>(*m_pScene);
                    m_pScene->SetScene(std::make_shared<Open3DScene>(w->GetRenderer()));
                    m_pScene->EnableSceneCaching(true); // smoother UI with large geometry
                    m_pScene->SetOnPointsPicked(
                        [this](const std::map<
                                   std::string,
                                   std::vector<std::pair<size_t, Eigen::Vector3d>>>
                                   &indices,
                               int keymods) {
                            if (keymods & int(KeyModifier::SHIFT))
                            {
                                m_sSelection->UnselectIndices(indices);
                            }
                            else
                            {
                                m_sSelection->SelectIndices(indices);
                            }
                        });
                    w->AddChild(GiveOwnership(m_pScene));

                    auto o3dscene = m_pScene->GetScene();
                    o3dscene->SetBackground(m_uiState.bg_color);

                    MakeSettingsUI();
                    SetMouseMode(m_uiState.m_mouseMode);
                    ShowAxes(true);
                    SetPointSize(m_uiState.m_pointSize); // sync selections_' point size
                    ResetCameraToDefault();
                    //                    SetPicking();

                    Eigen::Vector3f sun_dir = {0, 0, 0};
                    o3dscene->SetLighting(Open3DScene::LightingProfile::NO_SHADOWS, sun_dir);
                }

                void MakeSettingsUI()
                {
                    auto em = m_pWindow->GetTheme().font_size;
                    auto half_em = int(std::round(0.5f * float(em)));
                    auto v_spacing = int(std::round(0.25 * float(em)));

                    settings.m_panelAll = new Vert(half_em);
                    m_pWindow->AddChild(GiveOwnership(settings.m_panelAll));

                    Margins margins(em, 0, half_em, 0);
                    Margins tabbed_margins(0, half_em, 0, 0);

                    // Scan
                    settings.m_panelScan = new CollapsableVert("SCAN", v_spacing, margins);
                    settings.m_panelAll->AddChild(GiveOwnership(settings.m_panelScan));

                    auto *btnResetCam = new Button("Reset Camera  ");
                    btnResetCam->SetOnClicked(
                        [this]() { this->ResetCameraToDefault(); });

                    auto *btnScanStart = new Button("Start New Scan");
                    btnScanStart->SetOnClicked(
                        [this]() {
                            SetMouseMode(m_uiState.m_mouseMode);
                        });

                    auto *btnSelect = new Button("Select Point  ");
                    btnSelect->SetOnClicked(
                        [this]() { SetPicking(); });

                    auto h = new Horiz(v_spacing);
                    h->AddChild(GiveOwnership(btnResetCam));
                    h->AddStretch();
                    settings.m_panelScan->AddChild(GiveOwnership(h));

                    h = new Horiz(v_spacing);
                    h->AddChild(GiveOwnership(btnScanStart));
                    h->AddStretch();
                    settings.m_panelScan->AddChild(GiveOwnership(h));

                    h = new Horiz(v_spacing);
                    h->AddChild(GiveOwnership(btnSelect));
                    h->AddStretch();
                    settings.m_panelScan->AddChild(GiveOwnership(h));

                    // Vertex selection
                    settings.m_panelVertexSet = new CollapsableVert("MEASURE", v_spacing, margins);
                    settings.m_panelAll->AddChild(GiveOwnership(settings.m_panelVertexSet));

                    settings.m_btnNewVertexSet = new SmallButton(" + ");
                    settings.m_btnNewVertexSet->SetOnClicked(
                        [this]() {
                            NewSelectionSet();
                        });
                    settings.m_btnDeleteVertexSet = new SmallButton(" - ");
                    settings.m_btnDeleteVertexSet->SetOnClicked(
                        [this]() {
                            int idx = settings.m_listSelectSet->GetSelectedIndex();
                            RemoveSelectionSet(idx);
                        });
                    settings.m_listSelectSet = new ListView();
                    settings.m_listSelectSet->SetOnValueChanged([this](const char *, bool) 
                    {
                        SelectSelectionSet(settings.m_listSelectSet->GetSelectedIndex());
                    });

                    auto *btnCalc = new Button("Calculate");
                    btnCalc->SetOnClicked([this]()
                    {
                        CalcSelectionArea();
                    });

                    h = new Horiz();
                    h->AddChild(GiveOwnership(btnCalc));
                    h->AddStretch();
                    settings.m_panelVertexSet->AddChild(GiveOwnership(h));

                    const char *selection_help = "Ctrl-click to select a point";
                    h = new Horiz();
                    h->AddStretch();
                    h->AddChild(std::make_shared<Label>(selection_help));
                    h->AddStretch();
                    settings.m_panelVertexSet->AddChild(GiveOwnership(h));

                    h = new Horiz(v_spacing);
                    h->AddChild(std::make_shared<Label>("Selection Sets"));
                    h->AddStretch();
                    h->AddChild(GiveOwnership(settings.m_btnNewVertexSet));
                    h->AddChild(GiveOwnership(settings.m_btnDeleteVertexSet));
                    settings.m_panelVertexSet->AddChild(GiveOwnership(h));

                    settings.m_panelVertexSet->AddChild(GiveOwnership(settings.m_listSelectSet));

                    // Scene controls
                    settings.m_panelSetting = new CollapsableVert("SETTING", v_spacing, margins);
                    settings.m_panelAll->AddChild(GiveOwnership(settings.m_panelSetting));
                    settings.m_sliderPointSize = new Slider(Slider::INT);
                    settings.m_sliderPointSize->SetLimits(1, 10);
                    settings.m_sliderPointSize->SetValue(m_uiState.m_pointSize);
                    settings.m_sliderPointSize->SetOnValueChanged([this](const double newValue) {
                        this->SetPointSize(int(newValue));
                    });

                    auto *grid = new VGrid(2, v_spacing);
                    settings.m_panelSetting->AddChild(GiveOwnership(grid));

                    grid->AddChild(std::make_shared<Label>("PointSize"));
                    grid->AddChild(GiveOwnership(settings.m_sliderPointSize));
                }

                void AddGeometry(const std::string &name,
                                 std::shared_ptr<geometry::Geometry3D> geom,
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

                        auto cloud = std::dynamic_pointer_cast<geometry::PointCloud>(geom);
                        auto lines = std::dynamic_pointer_cast<geometry::LineSet>(geom);
                        auto mesh = std::dynamic_pointer_cast<geometry::MeshBase>(geom);

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
                    UpdateGeometryVisibility(m_vObject.back());

                    SetPointSize(m_uiState.m_pointSize);
                    m_pScene->ForceRedraw();
                }

                void AddTgeometry(const std::string &name,
                                   std::shared_ptr<t::geometry::PointCloud> tgeom,
                                   bool bVisible = true)
                {
                    bool no_shadows = false;
                    Material mat;
                    bool has_colors = false;
                    bool has_normals = false;

                    auto t_cloud = std::dynamic_pointer_cast<t::geometry::PointCloud>(tgeom);
                    auto t_mesh = std::dynamic_pointer_cast<t::geometry::TriangleMesh>(tgeom);
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
                    UpdateGeometryVisibility(m_vObject.back());

                    SetPointSize(m_uiState.m_pointSize);
                    m_pScene->ForceRedraw();
                }

                void RemoveGeometry(const std::string &name)
                {
                    for (size_t i = 0; i < m_vObject.size(); ++i)
                    {
                        if (m_vObject[i].m_name == name)
                        {
                            m_vObject.erase(m_vObject.begin() + i);
                            break;
                        }
                    }

                    m_pScene->GetScene()->RemoveGeometry(name);

                    // Bounds have changed, so update the selection point size, since they
                    // depend on the bounds.
                    SetPointSize(m_uiState.m_pointSize);

                    m_pScene->ForceRedraw();
                }

                void ShowGeometry(const std::string &name, bool show)
                {
                    for (auto &o : m_vObject)
                    {
                        if (o.m_name == name)
                        {
                            if (show != o.m_bVisible)
                            {
                                o.m_bVisible = show;

                                UpdateGeometryVisibility(o); // calls ForceRedraw()
                                m_pWindow->PostRedraw();

                                if (m_sSelection->IsActive())
                                {
                                    UpdateSelectableGeometry();
                                }
                                else
                                {
                                    bUpdateSelection = true;
                                }
                            }
                            break;
                        }
                    }
                }

                PCscanUI::DrawObject GetGeometry(const std::string &name) const
                {
                    for (auto &o : m_vObject)
                    {
                        if (o.m_name == name)
                        {
                            return o;
                        }
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

                void ResetCameraToDefault()
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
                                   std::shared_ptr<geometry::Image> bg_image)
                {
                    m_uiState.bg_color = bg_color;
                    auto scene = m_pScene->GetScene();
                    scene->SetBackground(m_uiState.bg_color, bg_image);
                    m_pScene->ForceRedraw();
                }

                void ShowSettings(bool show)
                {
                    m_uiState.m_bShowSettings = show;
                    settings.m_panelAll->SetVisible(show);
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
                    settings.m_sliderPointSize->SetValue(double(px));

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
                    auto psize = double(std::max(5, px)) * 0.000666 * std::max(xdim, std::max(ydim, zdim));
                    m_sSelection->SetPointSize(psize);

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

                void SetShader(PCscanUI::Shader shader)
                {
                    m_uiState.m_sceneShader = shader;
                    for (auto &o : m_vObject)
                    {
                        OverrideMaterial(o.m_name, o.m_material, shader);
                    }
                    m_pScene->ForceRedraw();
                }

                void OverrideMaterial(const std::string &name,
                                      const Material &original_material,
                                      PCscanUI::Shader shader)
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
                    return std::round(px * m_pWindow->GetScaling());
                }

                void SetMouseMode(SceneWidget::Controls mode)
                {
                    if (m_sSelection->IsActive())
                    {
                        m_sSelection->MakeInactive();
                    }

                    m_pScene->SetViewControls(mode);
                }

                void SetPicking()
                {
                    if (m_sSelection->GetNumberOfSets() == 0)
                    {
                        NewSelectionSet();
                    }
                    if (bUpdateSelection)
                    {
                        UpdateSelectableGeometry();
                    }
                    m_sSelection->MakeActive();
                }

                std::vector<O3DVisualizerSelections::SelectionSet> GetSelectionSets() const
                {
                    return m_sSelection->GetSets();
                }

                void SetUIState(const UIState &new_state)
                {
                    int point_size_changed = (new_state.m_pointSize != m_uiState.m_pointSize);
                    int line_width_changed = (new_state.m_lineWidth != m_uiState.m_lineWidth);
                    if (&new_state != &m_uiState)
                    {
                        m_uiState = new_state;
                    }

                    ShowSettings(m_uiState.m_bShowSettings);
                    SetShader(m_uiState.m_sceneShader);
                    SetBackground(m_uiState.bg_color, nullptr);
                    ShowAxes(m_uiState.m_bShowAxes);

                    if (point_size_changed)
                    {
                        SetPointSize(m_uiState.m_pointSize);
                    }
                    if (line_width_changed)
                    {
                        SetLineWidth(m_uiState.m_lineWidth);
                    }

                    m_pScene->ForceRedraw();
                }

                void UpdateGeometryVisibility(const DrawObject &o)
                {
                    m_pScene->GetScene()->ShowGeometry(o.m_name, o.m_bVisible);
                    m_pScene->ForceRedraw();
                }

                void NewSelectionSet()
                {
                    m_sSelection->NewSet();
                    UpdateSelectionSetList();
                    SelectSelectionSet(int(m_sSelection->GetNumberOfSets()) - 1);
                }

                void RemoveSelectionSet(int index)
                {
                    m_sSelection->RemoveSet(index);
                    if (m_sSelection->GetNumberOfSets() == 0)
                    {
                        // You can remove the last set, but there must always be one
                        // set, so we re-create one. (So removing the last set has the
                        // effect of clearing it.)
                        m_sSelection->NewSet();
                    }
                    UpdateSelectionSetList();
                }

                void SelectSelectionSet(int index)
                {
                    settings.m_listSelectSet->SetSelectedIndex(index);
                    m_sSelection->SelectSet(index);

                    //draw rectangle
                    std::map<std::string, std::set<O3DVisualizerSelections::SelectedIndex>> msSI;
                    msSI = m_sSelection->GetSets().at(index);

                    std::set<O3DVisualizerSelections::SelectedIndex> sSI = msSI["GEO"];
                    int nP = sSI.size();
                    if(nP < 4)return;

                    static geometry::LineSet LS;
                    LS.points_.clear();
                    LS.lines_.clear();
                    LS.colors_.clear();

                    for(O3DVisualizerSelections::SelectedIndex sI : sSI)
                    {
                        LS.points_.push_back(sI.point);
                    }

                    LS.lines_.push_back(Eigen::Vector2i(0,1));
                    LS.lines_.push_back(Eigen::Vector2i(1,2));
                    LS.lines_.push_back(Eigen::Vector2i(2,3));
                    LS.lines_.push_back(Eigen::Vector2i(3,0));

                    std::shared_ptr<geometry::LineSet> sLS = std::make_shared<geometry::LineSet>(LS);
                    AddGeometry("RectSelected", sLS);
                    m_pWindow->PostRedraw();
                }

                void UpdateSelectionSetList()
                {
                    size_t n = m_sSelection->GetNumberOfSets();
                    int idx = settings.m_listSelectSet->GetSelectedIndex();
                    idx = std::max(0, idx);
                    idx = std::min(idx, int(n) - 1);

                    std::vector<std::string> items;
                    items.reserve(n);
                    for (size_t i = 0; i < n; ++i)
                    {
                        std::stringstream s;
                        s << "Set " << (i + 1);
                        items.push_back(s.str());
                    }
                    settings.m_listSelectSet->SetItems(items);

                    SelectSelectionSet(idx);
                    m_pWindow->PostRedraw();
                }

                void CalcSelectionArea(void)
                {

                }

                void UpdateSelectableGeometry()
                {
                    std::vector<SceneWidget::PickableGeometry> pickable;
                    pickable.reserve(m_vObject.size());
                    for (auto &o : m_vObject)
                    {
                        if (!o.m_bVisible)
                            continue;

                        pickable.emplace_back(o.m_name, o.m_sGeometry.get(), o.m_sTgeometry.get());
                    }
                    m_sSelection->SetSelectableGeometry(pickable);
                }

                void ExportCurrentImage(const std::string &path)
                {
                    m_pScene->EnableSceneCaching(false);
                    m_pScene->GetScene()->GetScene()->RenderToImage(
                        [this, path](std::shared_ptr<geometry::Image> image) mutable {
                            if (!io::WriteImage(path, *image))
                            {
                                this->m_pWindow->ShowMessageBox(
                                    "Error",
                                    (std::string("Could not write image to ") +
                                     path + ".")
                                        .c_str());
                            }
                            m_pScene->EnableSceneCaching(true);
                        });
                }

                void OnExportRGB()
                {
                    auto dlg = std::make_shared<gui::FileDialog>(
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
            PCscanUI::PCscanUI(const std::string &title, int width, int height)
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
                std::shared_ptr<geometry::Image> bg_image /*= nullptr*/)
            {
                impl_->SetBackground(bg_color, bg_image);
            }

            void PCscanUI::AddGeometry(const std::string &name,
                                       std::shared_ptr<geometry::Geometry3D> geom,
                                       rendering::Material *material /*= nullptr*/,
                                       bool bVisible /*= true*/)
            {
                impl_->AddGeometry(name, geom, material, bVisible);
            }

            void PCscanUI::AddPointCloud(const std::string &name,
                                         std::shared_ptr<geometry::PointCloud> sPC,
                                         rendering::Material *material /*= nullptr*/,
                                         bool bVisible /*= true*/)
            {
                t::geometry::PointCloud tpcd = open3d::t::geometry::PointCloud::
                    FromLegacyPointCloud(
                        *sPC.get(),
                        core::Dtype::Float32);

                std::shared_ptr<t::geometry::PointCloud> spP =
                    std::make_shared<t::geometry::PointCloud>(tpcd);

                impl_->AddTgeometry(name, spP, bVisible);
            }

            void PCscanUI::UpdatePointCloud(const std::string &name,
                                            std::shared_ptr<geometry::PointCloud> sPC)
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

            void PCscanUI::RemoveGeometry(const std::string &name)
            {
                return impl_->RemoveGeometry(name);
            }

            void PCscanUI::ShowGeometry(const std::string &name, bool show)
            {
                return impl_->ShowGeometry(name, show);
            }

            PCscanUI::DrawObject PCscanUI::GetGeometry(const std::string &name) const
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

            std::vector<O3DVisualizerSelections::SelectionSet>
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

            PCscanUI::UIState PCscanUI::GetUIState() const
            {
                return impl_->m_uiState;
            }

            void PCscanUI::ExportCurrentImage(const std::string &path)
            {
                impl_->ExportCurrentImage(path);
            }

            void PCscanUI::Layout(const Theme &theme)
            {
                auto em = theme.font_size;
                int settings_width = 15 * theme.font_size;

                auto f = GetContentRect();
                if (impl_->settings.m_panelAll->IsVisible())
                {
                    impl_->m_pScene->SetFrame(Rect(f.x, f.y, f.width - settings_width, f.height));
                    impl_->settings.m_panelAll->SetFrame(Rect(f.GetRight() - settings_width, f.y, settings_width, f.height));
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