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
                std::set<std::string> added_names_;
                std::vector<DrawObject> objects_;
                std::shared_ptr<O3DVisualizerSelections> selections_;
                bool selections_need_update_ = true;

                UIState ui_state_;
                Window *window_ = nullptr;
                SceneWidget *scene_ = nullptr;

                struct
                {
                    Vert *panel;
                    CollapsableVert *mouse_panel;
                    Button *new_selection_set;
                    Button *delete_selection_set;
                    ListView *selection_sets;

                    CollapsableVert *scene_panel;
                    Slider *point_size;
                } settings;

                void Construct(PCscanUI *w)
                {
                    if (window_)
                        return;

                    window_ = w;
                    scene_ = new SceneWidget();
                    selections_ = std::make_shared<O3DVisualizerSelections>(*scene_);
                    scene_->SetScene(std::make_shared<Open3DScene>(w->GetRenderer()));
                    scene_->EnableSceneCaching(true); // smoother UI with large geometry
                    scene_->SetOnPointsPicked(
                        [this](const std::map<
                                   std::string,
                                   std::vector<std::pair<size_t, Eigen::Vector3d>>>
                                   &indices,
                               int keymods) {
                            if (keymods & int(KeyModifier::SHIFT))
                            {
                                selections_->UnselectIndices(indices);
                            }
                            else
                            {
                                selections_->SelectIndices(indices);
                            }
                        });
                    w->AddChild(GiveOwnership(scene_));

                    auto o3dscene = scene_->GetScene();
                    o3dscene->SetBackground(ui_state_.bg_color);

                    MakeSettingsUI();
//                    SetMouseMode(SceneWidget::Controls::ROTATE_CAMERA);
                    SetMouseMode(SceneWidget::Controls::FLY);
                    ShowAxes(true);
                    SetPointSize(ui_state_.point_size);       // sync selections_' point size
                    ResetCameraToDefault();
//                    SetPicking();

                    Eigen::Vector3f sun_dir = {0,0,0};
                    o3dscene->SetLighting(Open3DScene::LightingProfile::NO_SHADOWS, sun_dir);
                }

                void MakeSettingsUI()
                {
                    auto em = window_->GetTheme().font_size;
                    auto half_em = int(std::round(0.5f * float(em)));
                    auto v_spacing = int(std::round(0.25 * float(em)));

                    settings.panel = new Vert(half_em);
                    window_->AddChild(GiveOwnership(settings.panel));

                    Margins margins(em, 0, half_em, 0);
                    Margins tabbed_margins(0, half_em, 0, 0);

                    settings.mouse_panel = new CollapsableVert("SCAN", v_spacing, margins);
                    settings.panel->AddChild(GiveOwnership(settings.mouse_panel));

                    auto *btnResetCam = new SmallButton("Reset Camera  ");
                    btnResetCam->SetOnClicked
                    (
                        [this]() { this->ResetCameraToDefault(); }
                    );

                    auto *btnScanStart = new SmallButton("Start New Scan");
                    btnScanStart->SetOnClicked
                    (
                        [this]() { 
//                            SetMouseMode(SceneWidget::Controls::ROTATE_CAMERA);
                            SetMouseMode(SceneWidget::Controls::FLY);
                        }
                    );

                    auto *btnSelect = new SmallButton("Select Point  ");
                    btnSelect->SetOnClicked
                    (
                        [this]() { SetPicking(); }
                    );


                    auto h = new Horiz(v_spacing);
                    h->AddChild(GiveOwnership(btnResetCam));
                    h->AddStretch();
                    settings.mouse_panel->AddChild(GiveOwnership(h));

                    h = new Horiz(v_spacing);
                    h->AddChild(GiveOwnership(btnScanStart));
                    h->AddStretch();
                    settings.mouse_panel->AddChild(GiveOwnership(h));

                    h = new Horiz(v_spacing);
                    h->AddChild(GiveOwnership(btnSelect));
                    h->AddStretch();
                    settings.mouse_panel->AddChild(GiveOwnership(h));


                    // Selection sets controls
                    settings.new_selection_set = new SmallButton(" + ");
                    settings.new_selection_set->SetOnClicked
                    (
                        [this]() {
                            NewSelectionSet(); 
                    });
                    settings.delete_selection_set = new SmallButton(" - ");
                    settings.delete_selection_set->SetOnClicked
                    (
                        [this]() {
                        int idx = settings.selection_sets->GetSelectedIndex();
                        RemoveSelectionSet(idx);
                    });
                    settings.selection_sets = new ListView();
                    settings.selection_sets->SetOnValueChanged([this](const char *, bool) 
                    {
                        SelectSelectionSet(settings.selection_sets->GetSelectedIndex());
                    });

                    const char *selection_help = "Ctrl-click to select a point";
                    h = new Horiz();
                    h->AddStretch();
                    h->AddChild(std::make_shared<Label>(selection_help));
                    h->AddStretch();
                    settings.mouse_panel->AddChild(GiveOwnership(h));
                    h = new Horiz(v_spacing);
                    h->AddChild(std::make_shared<Label>("Selection Sets"));
                    h->AddStretch();
                    h->AddChild(GiveOwnership(settings.new_selection_set));
                    h->AddChild(GiveOwnership(settings.delete_selection_set));
                    settings.mouse_panel->AddChild(GiveOwnership(h));
                    settings.mouse_panel->AddChild(GiveOwnership(settings.selection_sets));


                    // Scene controls
                    settings.scene_panel = new CollapsableVert("Scene", v_spacing, margins);
                    settings.panel->AddChild(GiveOwnership(settings.scene_panel));
                    settings.point_size = new Slider(Slider::INT);
                    settings.point_size->SetLimits(1, 10);
                    settings.point_size->SetValue(ui_state_.point_size);
                    settings.point_size->SetOnValueChanged([this](const double newValue)
                    {
                        this->SetPointSize(int(newValue));
                    });

                    auto *grid = new VGrid(2, v_spacing);
                    settings.scene_panel->AddChild(GiveOwnership(grid));

                    grid->AddChild(std::make_shared<Label>("PointSize"));
                    grid->AddChild(GiveOwnership(settings.point_size));

                }

                void AddGeometry(const std::string &name,
                                 std::shared_ptr<geometry::Geometry3D> geom,
//                                 std::shared_ptr<t::geometry::Geometry> tgeom,
                                 std::shared_ptr<t::geometry::PointCloud> tgeom,
                                 rendering::Material *material,
                                 bool is_visible = true)
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

                        auto t_cloud = std::dynamic_pointer_cast<t::geometry::PointCloud>(tgeom);
                        auto t_mesh = std::dynamic_pointer_cast<t::geometry::TriangleMesh>(tgeom);

                        if (cloud)
                        {
                            has_colors = !cloud->colors_.empty();
                            has_normals = !cloud->normals_.empty();
                        }
                        else if (t_cloud)
                        {
                            has_colors = t_cloud->HasPointColors();
                            has_normals = t_cloud->HasPointNormals();
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
                        else if (t_mesh)
                        {
                            has_normals = !t_mesh->HasVertexNormals();
                            has_colors = true; // always want base_color as white
                        }

                        mat.base_color = CalcDefaultUnlitColor();
                        mat.shader = kShaderUnlit;
                        if (lines)
                        {
                            mat.shader = kShaderUnlitLines;
                            mat.line_width = ui_state_.line_width * window_->GetScaling();
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
                        mat.point_size = ConvertToScaledPixels(ui_state_.point_size);
                    }

                    objects_.push_back({name, geom, tgeom, mat, 
                                        is_visible, is_default_color});

                    auto scene = scene_->GetScene();

                    scene->AddGeometry(name, tgeom.get(), mat);
//                    scene->AddGeometry(name, geom.get(), mat);
                    
                    scene->GetScene()->GeometryShadows(name, false, false);
                    UpdateGeometryVisibility(objects_.back());

                    // Bounds have changed, so update the selection point size, since they
                    // depend on the bounds.
                    SetPointSize(ui_state_.point_size);

                    scene_->ForceRedraw();
                }

                void RemoveGeometry(const std::string &name)
                {
                    for (size_t i = 0; i < objects_.size(); ++i)
                    {
                        if (objects_[i].name == name)
                        {
                            objects_.erase(objects_.begin() + i);
                            break;
                        }
                    }

                    scene_->GetScene()->RemoveGeometry(name);

                    // Bounds have changed, so update the selection point size, since they
                    // depend on the bounds.
                    SetPointSize(ui_state_.point_size);

                    scene_->ForceRedraw();
                }

                void ShowGeometry(const std::string &name, bool show)
                {
                    for (auto &o : objects_)
                    {
                        if (o.name == name)
                        {
                            if (show != o.is_visible)
                            {
                                o.is_visible = show;

                                UpdateGeometryVisibility(o); // calls ForceRedraw()
                                window_->PostRedraw();

                                if (selections_->IsActive())
                                {
                                    UpdateSelectableGeometry();
                                }
                                else
                                {
                                    selections_need_update_ = true;
                                }
                            }
                            break;
                        }
                    }
                }

                PCscanUI::DrawObject GetGeometry(const std::string &name) const
                {
                    for (auto &o : objects_)
                    {
                        if (o.name == name)
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
                    auto scene = scene_->GetScene();
                    scene_->SetupCamera(fov, scene->GetBoundingBox(), {0.0f, 0.0f, 0.0f});
                    scene->GetCamera()->LookAt(center, eye, up);
                    scene_->ForceRedraw();
                }

                void ResetCameraToDefault()
                {
                    auto scene = scene_->GetScene();
                    scene_->SetupCamera(60.0f, scene->GetBoundingBox(), {0.0f, 0.0f, 0.0f});

                    Eigen::Vector3f vCenter(0,0,0);
                    Eigen::Vector3f vEye(0,0,1);
                    Eigen::Vector3f vUp(1,0,0);
                    scene->GetCamera()->LookAt(vCenter, vEye, vUp);

                    scene_->ForceRedraw();
                }

                void SetBackground(const Eigen::Vector4f &bg_color,
                                   std::shared_ptr<geometry::Image> bg_image)
                {
                    ui_state_.bg_color = bg_color;
                    auto scene = scene_->GetScene();
                    scene->SetBackground(ui_state_.bg_color, bg_image);
                    scene_->ForceRedraw();
                }

                void ShowSettings(bool show)
                {
                    ui_state_.show_settings = show;
                    settings.panel->SetVisible(show);
                    window_->SetNeedsLayout();
                }

                void ShowAxes(bool show)
                {
                    ui_state_.show_axes = show;
                    scene_->GetScene()->ShowAxes(show);
                    scene_->ForceRedraw();
                }

                void SetPointSize(int px)
                {
                    ui_state_.point_size = px;
                    settings.point_size->SetValue(double(px));

                    px = int(ConvertToScaledPixels(px));
                    for (auto &o : objects_)
                    {
                        o.material.point_size = float(px);
                        OverrideMaterial(o.name, o.material, ui_state_.scene_shader);
                    }
                    auto bbox = scene_->GetScene()->GetBoundingBox();
                    auto xdim = bbox.max_bound_.x() - bbox.min_bound_.x();
                    auto ydim = bbox.max_bound_.y() - bbox.min_bound_.z();
                    auto zdim = bbox.max_bound_.z() - bbox.min_bound_.y();
                    auto psize = double(std::max(5, px)) * 0.000666 * std::max(xdim, std::max(ydim, zdim));
                    selections_->SetPointSize(psize);

                    scene_->SetPickablePointSize(px);
                    scene_->ForceRedraw();
                }

                void SetLineWidth(int px)
                {
                    ui_state_.line_width = px;

                    px = int(ConvertToScaledPixels(px));
                    for (auto &o : objects_)
                    {
                        o.material.line_width = float(px);
                        OverrideMaterial(o.name, o.material, ui_state_.scene_shader);
                    }
                    scene_->ForceRedraw();
                }

                void SetShader(PCscanUI::Shader shader)
                {
                    ui_state_.scene_shader = shader;
                    for (auto &o : objects_)
                    {
                        OverrideMaterial(o.name, o.material, shader);
                    }
                    scene_->ForceRedraw();
                }

                void OverrideMaterial(const std::string &name,
                                      const Material &original_material,
                                      PCscanUI::Shader shader)
                {
                    bool is_lines = (original_material.shader == "unlitLine" ||
                                     original_material.shader == "lines");
                    auto scene = scene_->GetScene();
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
                    return std::round(px * window_->GetScaling());
                }

                void SetMouseMode(SceneWidget::Controls mode)
                {
                    if (selections_->IsActive())
                    {
                        selections_->MakeInactive();
                    }

                    scene_->SetViewControls(mode);
                }

                void SetPicking()
                {
                    if (selections_->GetNumberOfSets() == 0)
                    {
                        NewSelectionSet();
                    }
                    if (selections_need_update_)
                    {
                        UpdateSelectableGeometry();
                    }
                    selections_->MakeActive();
                }

                std::vector<O3DVisualizerSelections::SelectionSet> GetSelectionSets() const
                {
                    return selections_->GetSets();
                }

                void SetUIState(const UIState &new_state)
                {
                    int point_size_changed = (new_state.point_size != ui_state_.point_size);
                    int line_width_changed = (new_state.line_width != ui_state_.line_width);
                    if (&new_state != &ui_state_)
                    {
                        ui_state_ = new_state;
                    }

                    ShowSettings(ui_state_.show_settings);
                    SetShader(ui_state_.scene_shader);
                    SetBackground(ui_state_.bg_color, nullptr);
                    ShowAxes(ui_state_.show_axes);

                    if (point_size_changed)
                    {
                        SetPointSize(ui_state_.point_size);
                    }
                    if (line_width_changed)
                    {
                        SetLineWidth(ui_state_.line_width);
                    }

                    scene_->ForceRedraw();
                }

                void UpdateGeometryVisibility(const DrawObject &o)
                {
                    scene_->GetScene()->ShowGeometry(o.name, o.is_visible);
                    scene_->ForceRedraw();
                }

                void NewSelectionSet()
                {
                    selections_->NewSet();
                    UpdateSelectionSetList();
                    SelectSelectionSet(int(selections_->GetNumberOfSets()) - 1);
                }

                void RemoveSelectionSet(int index)
                {
                    selections_->RemoveSet(index);
                    if (selections_->GetNumberOfSets() == 0)
                    {
                        // You can remove the last set, but there must always be one
                        // set, so we re-create one. (So removing the last set has the
                        // effect of clearing it.)
                        selections_->NewSet();
                    }
                    UpdateSelectionSetList();
                }

                void SelectSelectionSet(int index)
                {
                    settings.selection_sets->SetSelectedIndex(index);
                    selections_->SelectSet(index);
                }

                void UpdateSelectionSetList()
                {
                    size_t n = selections_->GetNumberOfSets();
                    int idx = settings.selection_sets->GetSelectedIndex();
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
                    settings.selection_sets->SetItems(items);
                    SelectSelectionSet(idx);
                    window_->PostRedraw();
                }

                void UpdateSelectableGeometry()
                {
                    std::vector<SceneWidget::PickableGeometry> pickable;
                    pickable.reserve(objects_.size());
                    for (auto &o : objects_)
                    {
                        if (!o.is_visible)
                            continue;

                        pickable.emplace_back(o.name, o.geometry.get(), o.tgeometry.get());
                    }
                    selections_->SetSelectableGeometry(pickable);
                }

                void ExportCurrentImage(const std::string &path)
                {
                    scene_->EnableSceneCaching(false);
                    scene_->GetScene()->GetScene()->RenderToImage(
                        [this, path](std::shared_ptr<geometry::Image> image) mutable {
                            if (!io::WriteImage(path, *image))
                            {
                                this->window_->ShowMessageBox(
                                    "Error",
                                    (std::string("Could not write image to ") +
                                     path + ".")
                                        .c_str());
                            }
                            scene_->EnableSceneCaching(true);
                        });
                }

                void OnExportRGB()
                {
                    auto dlg = std::make_shared<gui::FileDialog>(
                        gui::FileDialog::Mode::SAVE, "Save File", window_->GetTheme());
                    dlg->AddFilter(".png", "PNG images (.png)");
                    dlg->AddFilter("", "All files");
                    dlg->SetOnCancel([this]() { this->window_->CloseDialog(); });
                    dlg->SetOnDone([this](const char *path) {
                        this->window_->CloseDialog();
                        this->ExportCurrentImage(path);
                    });
                    window_->ShowDialog(dlg);
                }

                Eigen::Vector4f CalcDefaultUnlitColor()
                {
                    float luminosity = 0.21f * ui_state_.bg_color.x() +
                                       0.72f * ui_state_.bg_color.y() +
                                       0.07f * ui_state_.bg_color.z();
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
                return impl_->scene_->GetScene().get();
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
                                       bool is_visible /*= true*/)
            {
                impl_->AddGeometry(name, geom, nullptr, material,
                 is_visible);
            }

            void PCscanUI::AddPointCloud(const std::string &name,
                                         std::shared_ptr<geometry::PointCloud> sPC,
                                         rendering::Material *material /*= nullptr*/,
                                          bool is_visible /*= true*/)
            {
                t::geometry::PointCloud tpcd = open3d::t::geometry::PointCloud::
                    FromLegacyPointCloud(
                        *sPC.get(),
                        core::Dtype::Float32);

                std::shared_ptr<t::geometry::PointCloud> spP = 
                    std::make_shared<t::geometry::PointCloud>(tpcd);

                impl_->AddGeometry(name,
                                    nullptr, 
                                    spP,
                                    material, 
                                    is_visible);
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

                        impl_->scene_->GetScene()->GetScene()->UpdateGeometry(
                            name,
                            tpcd,
                            open3d::visualization::rendering::Scene::kUpdatePointsFlag |
                                open3d::visualization::rendering::Scene::kUpdateColorsFlag);
                        impl_->scene_->ForceRedraw();
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
                return impl_->ui_state_;
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
                if (impl_->settings.panel->IsVisible())
                {
                    impl_->scene_->SetFrame(Rect(f.x, f.y, f.width - settings_width, f.height));
                    impl_->settings.panel->SetFrame(Rect(f.GetRight() - settings_width, f.y, settings_width, f.height));
                }
                else
                {
                    impl_->scene_->SetFrame(f);
                }

                Super::Layout(theme);
            }

        } // namespace visualizer
    }     // namespace visualization
} // namespace open3d

#endif