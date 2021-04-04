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

                class DrawObjectTreeCell : public Widget
                {
                    using Super = Widget;

                public:
                    enum
                    {
                        FLAG_NONE = 0,
                        FLAG_GROUP = (1 << 0),
                        FLAG_TIME = (1 << 1)
                    };

                    DrawObjectTreeCell(const char *name,
                                       const char *group,
                                       double time,
                                       bool is_checked,
                                       int flags,
                                       std::function<void(bool)> on_toggled)
                    {
                        flags_ = flags;

                        // We don't want any text in the checkbox, but passing "" seems to make
                        // it not toggle, so we need to pass in something. This way it will
                        // just be extra spacing.
                        checkbox_ = std::make_shared<Checkbox>(" ");
                        checkbox_->SetChecked(is_checked);
                        checkbox_->SetOnChecked(on_toggled);
                        name_ = std::make_shared<Label>(name);
                        group_ = std::make_shared<Label>((flags & FLAG_GROUP) ? group : "");
                        AddChild(checkbox_);
                        AddChild(name_);
                        AddChild(group_);
                    }

                    ~DrawObjectTreeCell() {}

                    std::shared_ptr<Checkbox> GetCheckbox() { return checkbox_; }
                    std::shared_ptr<Label> GetName() { return name_; }

                    Size CalcPreferredSize(const Theme &theme) const override
                    {
                        auto check_pref = checkbox_->CalcPreferredSize(theme);
                        auto name_pref = name_->CalcPreferredSize(theme);
                        int w = check_pref.width + name_pref.width + GroupWidth(theme);
                        return Size(w, std::max(check_pref.height, name_pref.height));
                    }

                    void Layout(const Theme &theme) override
                    {
                        auto &frame = GetFrame();
                        auto check_width = checkbox_->CalcPreferredSize(theme).width;
                        checkbox_->SetFrame(Rect(frame.x, frame.y, check_width, frame.height));
                        auto group_width = GroupWidth(theme);
                        auto x = checkbox_->GetFrame().GetRight();
                        auto name_width = frame.GetRight() - group_width - x;
                        name_->SetFrame(Rect(x, frame.y, name_width, frame.height));
                        x += name_width;
                        group_->SetFrame(Rect(x, frame.y, group_width, frame.height));
                        x += group_width;
                    }

                private:
                    int flags_;
                    std::shared_ptr<Checkbox> checkbox_;
                    std::shared_ptr<Label> name_;
                    std::shared_ptr<Label> group_;

                    int GroupWidth(const Theme &theme) const
                    {
                        if (flags_ & FLAG_GROUP)
                            return 5 * theme.font_size;
                        else
                            return 0;
                    }
                };

            } // namespace

            struct PCscanUI::Impl
            {
                std::set<std::string> added_names_;
                std::set<std::string> added_groups_;
                std::vector<DrawObject> objects_;
                std::shared_ptr<O3DVisualizerSelections> selections_;
                bool selections_need_update_ = true;

                UIState ui_state_;
                Window *window_ = nullptr;
                SceneWidget *scene_ = nullptr;

                struct
                {
                    // We only keep pointers here because that way we don't have to release
                    // all the shared_ptrs at destruction just to ensure that the gui gets
                    // destroyed before the Window, because the Window will do that for us.
                    Menu *actions_menu;
                    std::unordered_map<int, std::function<void(PCscanUI &)>> menuid2action;

                    Vert *panel;
                    CollapsableVert *mouse_panel;
                    TabControl *mouse_tab;
                    Vert *view_panel;
                    SceneWidget::Controls view_mouse_mode;
                    std::map<SceneWidget::Controls, Button *> mouse_buttons;
                    Vert *pick_panel;
                    Button *new_selection_set;
                    Button *delete_selection_set;
                    ListView *selection_sets;

                    CollapsableVert *scene_panel;
                    Slider *point_size;

                    CollapsableVert *geometries_panel;
                    TreeView *geometries;
                    std::map<std::string, TreeView::ItemId> group2itemid;
                    std::map<std::string, TreeView::ItemId> object2itemid;

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
                    SetMouseMode(SceneWidget::Controls::ROTATE_CAMERA);
                    ShowAxes(true);
                    SetPointSize(ui_state_.point_size);       // sync selections_' point size
                    ResetCameraToDefault();

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

                    settings.mouse_panel = new CollapsableVert("Mouse Controls", v_spacing, margins);
                    settings.panel->AddChild(GiveOwnership(settings.mouse_panel));

                    settings.mouse_tab = new TabControl();
                    settings.mouse_panel->AddChild(GiveOwnership(settings.mouse_tab));

                    settings.view_panel = new Vert(v_spacing, tabbed_margins);
                    settings.pick_panel = new Vert(v_spacing, tabbed_margins);
                    settings.mouse_tab->AddTab("Scene", GiveOwnership(settings.view_panel));
                    settings.mouse_tab->AddTab("Selection", GiveOwnership(settings.pick_panel));
                    settings.mouse_tab->SetOnSelectedTabChanged([this](int tab_idx)
                    {
                        if (tab_idx == 0)
                        {
                            SetMouseMode(settings.view_mouse_mode);
                        }
                        else
                        {
                            SetPicking();
                        }
                    });

                    // Mouse countrols
                    auto MakeMouseButton = [this](const char *name, SceneWidget::Controls type)
                    {
                        auto button = new SmallToggleButton(name);
                        button->SetOnClicked([this, type]() { this->SetMouseMode(type); });
                        this->settings.mouse_buttons[type] = button;
                        return button;
                    };
                    auto *h = new Horiz(v_spacing);
                    h->AddStretch();
                    h->AddChild(GiveOwnership(MakeMouseButton("Arcball", SceneWidget::Controls::ROTATE_CAMERA)));
                    h->AddStretch();
                    settings.view_panel->AddChild(GiveOwnership(h));
                    settings.view_panel->AddFixed(half_em);

                    auto *reset = new SmallButton("Reset Camera");
                    reset->SetOnClicked([this]() { this->ResetCameraToDefault(); });

                    h = new Horiz(v_spacing);
                    h->AddStretch();
                    h->AddChild(GiveOwnership(reset));
                    h->AddStretch();
                    settings.view_panel->AddChild(GiveOwnership(h));

                    // Selection sets controls
                    settings.new_selection_set = new SmallButton(" + ");
                    settings.new_selection_set->SetOnClicked(
                        [this]() { NewSelectionSet(); });
                    settings.delete_selection_set = new SmallButton(" - ");
                    settings.delete_selection_set->SetOnClicked([this]() {
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
                    settings.pick_panel->AddChild(GiveOwnership(h));
                    h = new Horiz(v_spacing);
                    h->AddChild(std::make_shared<Label>("Selection Sets"));
                    h->AddStretch();
                    h->AddChild(GiveOwnership(settings.new_selection_set));
                    h->AddChild(GiveOwnership(settings.delete_selection_set));
                    settings.pick_panel->AddChild(GiveOwnership(h));
                    settings.pick_panel->AddChild(GiveOwnership(settings.selection_sets));

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

                    // Geometry list
                    settings.geometries_panel =
                        new CollapsableVert("Geometries", v_spacing, margins);
                    settings.panel->AddChild(GiveOwnership(settings.geometries_panel));

                    settings.geometries = new TreeView();
                    settings.geometries_panel->AddChild(GiveOwnership(settings.geometries));

                }

                void AddGeometry(const std::string &name,
                                 std::shared_ptr<geometry::Geometry3D> geom,
//                                 std::shared_ptr<t::geometry::Geometry> tgeom,
                                 std::shared_ptr<t::geometry::PointCloud> tgeom,
                                 rendering::Material *material,
                                 const std::string &group,
                                 double time,
                                 bool is_visible)
                {
                    std::string group_name = group;
                    if (group_name == "")
                    {
                        group_name = "default";
                    }
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

                    // We assume that the caller isn't setting a group or time (and in any
                    // case we don't know beforehand what they will do). So if they do,
                    // we need to update the geometry tree accordingly. This needs to happen
                    // before we add the object to the list, otherwise when we regenerate
                    // the object will already be added in the list and then get added again
                    // below.
                    AddGroup(group_name); // regenerates if necessary

                    objects_.push_back({name, geom, tgeom, mat, group_name, time,
                                        is_visible, is_default_color});
                    AddObjectToTree(objects_.back());

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
                    std::string group;
                    for (size_t i = 0; i < objects_.size(); ++i)
                    {
                        if (objects_[i].name == name)
                        {
                            group = objects_[i].group;
                            objects_.erase(objects_.begin() + i);
                            settings.object2itemid.erase(objects_[i].name);
                            break;
                        }
                    }

                    // Need to check group membership in case this was the last item in its
                    // group. As long as we're doing that, recompute the min/max time, too.
                    std::set<std::string> groups;
                    for (size_t i = 0; i < objects_.size(); ++i)
                    {
                        auto &o = objects_[i];
                        groups.insert(o.group);
                    }

                    added_groups_ = groups;
                    std::set<std::string> enabled;
                    for (auto &g : ui_state_.enabled_groups)
                    { // remove deleted groups
                        if (groups.find(g) != groups.end())
                        {
                            enabled.insert(g);
                        }
                    }
                    ui_state_.enabled_groups = enabled;
                    UpdateObjectTree();
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

                                auto id = settings.object2itemid[o.name];
                                auto cell = settings.geometries->GetItem(id);
                                auto obj_cell =
                                    std::dynamic_pointer_cast<DrawObjectTreeCell>(cell);
                                if (obj_cell)
                                {
                                    obj_cell->GetCheckbox()->SetChecked(show);
                                }

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
                    settings.view_mouse_mode = mode;
                    for (const auto &t_b : settings.mouse_buttons)
                    {
                        t_b.second->SetOn(false);
                    }
                    settings.mouse_buttons[mode]->SetOn(true);
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
                    auto old_enabled_groups = ui_state_.enabled_groups;
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

                    if (old_enabled_groups != ui_state_.enabled_groups)
                    {
                        for (auto &group : added_groups_)
                        {
                            bool enabled = (ui_state_.enabled_groups.find(group) !=
                                            ui_state_.enabled_groups.end());
                            EnableGroup(group, enabled);
                        }
                    }

                    scene_->ForceRedraw();
                }

                void AddGroup(const std::string &group)
                {
                    if (added_groups_.find(group) == added_groups_.end())
                    {
                        added_groups_.insert(group);
                        ui_state_.enabled_groups.insert(group);
                    }
                    if (added_groups_.size() == 2)
                    {
                        UpdateObjectTree();
                    }
                }

                void EnableGroup(const std::string &group, bool enable)
                {
                    auto group_it = settings.group2itemid.find(group);
                    if (group_it != settings.group2itemid.end())
                    {
                        auto cell = settings.geometries->GetItem(group_it->second);
                        auto group_cell =
                            std::dynamic_pointer_cast<CheckableTextTreeCell>(cell);
                        if (group_cell)
                        {
                            group_cell->GetCheckbox()->SetChecked(enable);
                        }
                    }

                    if (enable)
                    {
                        ui_state_.enabled_groups.insert(group);
                    }
                    else
                    {
                        ui_state_.enabled_groups.erase(group);
                    }
                    for (auto &o : objects_)
                    {
                        UpdateGeometryVisibility(o);
                    }
                }

                void AddObjectToTree(const DrawObject &o)
                {
                    TreeView::ItemId parent = settings.geometries->GetRootItem();

                    if (added_groups_.size() >= 2)
                    {
                        auto it = settings.group2itemid.find(o.group);
                        if (it != settings.group2itemid.end())
                        {
                            parent = it->second;
                        }
                        else
                        {
                            auto cell = std::make_shared<CheckableTextTreeCell>(
                                o.group.c_str(), true,
                                [this, group = o.group](bool is_on) {
                                    this->EnableGroup(group, is_on);
                                });
                            parent = settings.geometries->AddItem(parent, cell);
                            settings.group2itemid[o.group] = parent;
                        }
                    }


                    int flag = DrawObjectTreeCell::FLAG_NONE;

                    auto cell = std::make_shared<DrawObjectTreeCell>(
                        o.name.c_str(), o.group.c_str(), o.time, o.is_visible, flag,
                        [this, name = o.name](bool is_on) {
                            ShowGeometry(name, is_on);
                        });
                    auto id = settings.geometries->AddItem(parent, cell);
                    settings.object2itemid[o.name] = id;
                }

                void UpdateObjectTree()
                {
                    settings.group2itemid.clear();
                    settings.object2itemid.clear();
                    settings.geometries->Clear();

                    for (auto &o : objects_)
                    {
                        AddObjectToTree(o);
                    }
                }

                void UpdateGeometryVisibility(const DrawObject &o)
                {
                    scene_->GetScene()->ShowGeometry(o.name, IsGeometryVisible(o));
                    scene_->ForceRedraw();
                }

                bool IsGeometryVisible(const DrawObject &o)
                {
                    bool is_group_enabled = (ui_state_.enabled_groups.find(o.group) !=
                                             ui_state_.enabled_groups.end());
                    bool is_visible = o.is_visible;
                    return (is_visible & is_group_enabled);
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
                        if (!IsGeometryVisible(o))
                        {
                            continue;
                        }
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

                std::string UniquifyName(const std::string &name)
                {
                    if (added_names_.find(name) == added_names_.end())
                    {
                        return name;
                    }

                    int n = 0;
                    std::string unique;
                    do
                    {
                        n += 1;
                        std::stringstream s;
                        s << name << "_" << n;
                        unique = s.str();
                    } while (added_names_.find(unique) != added_names_.end());

                    return unique;
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
                                       const std::string &group /*= ""*/,
                                       double time /*= 0.0*/,
                                       bool is_visible /*= true*/)
            {
                impl_->AddGeometry(name, geom, nullptr, material, group, time, is_visible);
            }

            void PCscanUI::AddPointCloud(const std::string &name,
                                         std::shared_ptr<geometry::PointCloud> sPC,
                                         rendering::Material *material /*= nullptr*/,
                                         const std::string &group /*= ""*/,
                                         double time /*= 0.0*/,
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
                                    material, group, time, is_visible);
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

            void PCscanUI::EnableGroup(const std::string &group, bool enable)
            {
                impl_->EnableGroup(group, enable);
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