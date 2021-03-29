#ifdef USE_OPEN3D
#include "WindowO3D.h"

namespace open3d
{
    namespace visualization
    {
        WindowO3D::WindowO3D(const std::string &title, int width, int height)
            : gui::Window(title, width, height), impl_(new WindowO3D::Impl())
        {
            Init();
        }

        WindowO3D::WindowO3D(
            const std::vector<std::shared_ptr<const geometry::Geometry>>
                &geometries,
            const std::string &title,
            int width,
            int height,
            int left,
            int top)
            : gui::Window(title, left, top, width, height),
              impl_(new WindowO3D::Impl())
        {
            Init();
            SetGeometry(geometries[0], false); // also updates the camera
        }

        void WindowO3D::Init()
        {
            auto &app = gui::Application::GetInstance();
            auto &theme = GetTheme();

            // Create menu
            if (!gui::Application::GetInstance().GetMenubar())
            {
                auto menu = std::make_shared<gui::Menu>();
                auto file_menu = std::make_shared<gui::Menu>();
                file_menu->AddItem("Open...", FILE_OPEN, gui::KEY_O);
                file_menu->AddItem("Export Current Image...", FILE_EXPORT_RGB);
                file_menu->AddSeparator();
                file_menu->AddItem("Quit", FILE_QUIT, gui::KEY_Q);
                menu->AddMenu("File", file_menu);

                auto settings_menu = std::make_shared<gui::Menu>();
                settings_menu->AddItem("Lighting & Materials",
                                       SETTINGS_LIGHT_AND_MATERIALS);
                settings_menu->SetChecked(SETTINGS_LIGHT_AND_MATERIALS, true);
                menu->AddMenu("Settings", settings_menu);

                auto help_menu = std::make_shared<gui::Menu>();
                help_menu->AddItem("Show Controls", HELP_KEYS);
                help_menu->AddItem("Show Camera Info", HELP_CAMERA);
                help_menu->AddSeparator();
                help_menu->AddItem("About", HELP_ABOUT);
                help_menu->AddItem("Contact", HELP_CONTACT);
                menu->AddMenu("Help", help_menu);

                gui::Application::GetInstance().SetMenubar(menu);
            }

            // Create scene
            impl_->scene_wgt_ = std::make_shared<gui::SceneWidget>();
            impl_->scene_wgt_->SetScene(std::make_shared<rendering::Open3DScene>(GetRenderer()));
            impl_->scene_wgt_->SetOnSunDirectionChanged(
                [this](const Eigen::Vector3f &new_dir) {
                    auto lighting = impl_->settings_.model_.GetLighting(); // copy
                    lighting.sun_dir = new_dir.normalized();
                    impl_->settings_.model_.SetCustomLighting(lighting);
                });
            impl_->scene_wgt_->EnableSceneCaching(true);

            // Create light
            auto &settings = impl_->settings_;
            std::string resource_path = app.GetResourcePath();
            auto ibl_path = resource_path + "/default";
            auto *render_scene = impl_->scene_wgt_->GetScene()->GetScene();
            render_scene->SetIndirectLight(ibl_path);
            impl_->scene_wgt_->ShowSkybox(settings.model_.GetShowSkybox());

            // Create materials
            impl_->InitializeMaterials(GetRenderer(), resource_path);

            // Apply model settings (which should be defaults) to the rendering entities
            impl_->UpdateFromModel(GetRenderer(), false);

            // Setup UI
            const auto em = theme.font_size;
            const int lm = int(std::ceil(0.5 * em));
            const int grid_spacing = int(std::ceil(0.25 * em));

            AddChild(impl_->scene_wgt_);

            // Add settings widget
            const int separation_height = int(std::ceil(0.75 * em));
            // (we don't want as much left margin because the twisty arrow is the
            // only thing there, and visually it looks larger than the right.)
            const gui::Margins base_margins(int(std::round(0.5 * lm)), lm, lm, lm);
            settings.wgt_base = std::make_shared<gui::Vert>(0, base_margins);

            gui::Margins indent(em, 0, 0, 0);
            auto view_ctrls =
                std::make_shared<gui::CollapsableVert>("Mouse controls", 0, indent);

            // ... view manipulator buttons
            settings.wgt_mouse_arcball = std::make_shared<SmallToggleButton>("Arcball");
            impl_->settings_.wgt_mouse_arcball->SetOn(true);
            settings.wgt_mouse_arcball->SetOnClicked([this]() {
                impl_->SetMouseControls(*this,
                                        gui::SceneWidget::Controls::ROTATE_CAMERA);
            });
            settings.wgt_mouse_fly = std::make_shared<SmallToggleButton>("Fly");
            settings.wgt_mouse_fly->SetOnClicked([this]() {
                impl_->SetMouseControls(*this, gui::SceneWidget::Controls::FLY);
            });
            settings.wgt_mouse_model = std::make_shared<SmallToggleButton>("Model");
            settings.wgt_mouse_model->SetOnClicked([this]() {
                impl_->SetMouseControls(*this,
                                        gui::SceneWidget::Controls::ROTATE_MODEL);
            });
            settings.wgt_mouse_sun = std::make_shared<SmallToggleButton>("Sun");
            settings.wgt_mouse_sun->SetOnClicked([this]() {
                impl_->SetMouseControls(*this, gui::SceneWidget::Controls::ROTATE_SUN);
            });
            settings.wgt_mouse_ibl = std::make_shared<SmallToggleButton>("Environment");
            settings.wgt_mouse_ibl->SetOnClicked([this]() {
                impl_->SetMouseControls(*this, gui::SceneWidget::Controls::ROTATE_IBL);
            });

            auto reset_camera = std::make_shared<SmallButton>("Reset camera");
            reset_camera->SetOnClicked([this]() {
                impl_->scene_wgt_->GoToCameraPreset(
                    gui::SceneWidget::CameraPreset::PLUS_Z);
            });

            auto camera_controls1 = std::make_shared<gui::Horiz>(grid_spacing);
            camera_controls1->AddStretch();
            camera_controls1->AddChild(settings.wgt_mouse_arcball);
            camera_controls1->AddChild(settings.wgt_mouse_fly);
            camera_controls1->AddChild(settings.wgt_mouse_model);
            camera_controls1->AddStretch();
            auto camera_controls2 = std::make_shared<gui::Horiz>(grid_spacing);
            camera_controls2->AddStretch();
            camera_controls2->AddChild(settings.wgt_mouse_sun);
            camera_controls2->AddChild(settings.wgt_mouse_ibl);
            camera_controls2->AddStretch();
            view_ctrls->AddChild(camera_controls1);
            view_ctrls->AddFixed(int(std::ceil(0.25 * em)));
            view_ctrls->AddChild(camera_controls2);
            view_ctrls->AddFixed(separation_height);
            view_ctrls->AddChild(gui::Horiz::MakeCentered(reset_camera));
            settings.wgt_base->AddChild(view_ctrls);

            // ... lighting and materials
            settings.view_ = std::make_shared<GuiSettingsView>(
                settings.model_, theme, resource_path, [this](const char *name) {
                    std::string resource_path =
                        gui::Application::GetInstance().GetResourcePath();
                    impl_->SetIBL(GetRenderer(),
                                  resource_path + "/" + name + "_ibl.ktx");
                });
            settings.model_.SetOnChanged([this](bool material_type_changed) {
                impl_->settings_.view_->Update();
                impl_->UpdateFromModel(GetRenderer(), material_type_changed);
            });
            settings.wgt_base->AddChild(settings.view_);

            AddChild(settings.wgt_base);
        }

        WindowO3D::~WindowO3D() {}

        void WindowO3D::SetTitle(const std::string &title)
        {
            Super::SetTitle(title.c_str());
        }

        void WindowO3D::SetGeometry(
            std::shared_ptr<const geometry::Geometry> geometry, bool loaded_model)
        {
            auto scene3d = impl_->scene_wgt_->GetScene();
            scene3d->ClearGeometry();

            impl_->SetMaterialsToDefault();

            rendering::Material loaded_material;
            if (loaded_model)
            {
                scene3d->AddModel(MODEL_NAME, impl_->loaded_model_);
                impl_->settings_.model_.SetDisplayingPointClouds(false);
            }
            else
            {
                // NOTE: If a model was NOT loaded then these must be point clouds
                std::shared_ptr<const geometry::Geometry> g = geometry;

                // If a point cloud or mesh has no vertex colors or a single uniform
                // color (usually white), then we want to display it normally, that
                // is, lit. But if the cloud/mesh has differing vertex colors, then
                // we assume that the vertex colors have the lighting value baked in
                // (for example, fountain.ply at http://qianyi.info/scenedata.html)
                if (g->GetGeometryType() ==
                    geometry::Geometry::GeometryType::PointCloud)
                {
                    auto pcd = std::static_pointer_cast<const geometry::PointCloud>(g);

                    //                    if (pcd->HasColors() && !PointCloudHasUniformColor(*pcd))
                    //                    {
                    loaded_material.shader = "defaultUnlit";
                    // }
                    // else
                    // {
                    //     loaded_material.shader = "defaultLit";
                    // }

                    //                    scene3d->AddGeometry(MODEL_NAME, pcd.get(), loaded_material);
                    t::geometry::PointCloud tpcd = open3d::t::geometry::PointCloud::FromLegacyPointCloud(*pcd.get(), core::Dtype::Float32);
                    scene3d->AddGeometry(MODEL_NAME,
                                         &tpcd,
                                         loaded_material);

                    impl_->settings_.model_.SetDisplayingPointClouds(true);
                    if (!impl_->settings_.model_.GetUserHasChangedLightingProfile())
                    {
                        auto &profile =
                            GuiSettingsModel::GetDefaultPointCloudLightingProfile();
                        impl_->settings_.model_.SetLightingProfile(profile);
                    }
                }
            }

            auto type = impl_->settings_.model_.GetMaterialType();
            if (type == GuiSettingsModel::MaterialType::LIT ||
                type == GuiSettingsModel::MaterialType::UNLIT)
            {
                if (loaded_material.shader == "defaultUnlit")
                {
                    impl_->settings_.model_.SetMaterialType(
                        GuiSettingsModel::MaterialType::UNLIT);
                }
                else
                {
                    impl_->settings_.model_.SetMaterialType(
                        GuiSettingsModel::MaterialType::LIT);
                }
            }

            // Setup UI for loaded model/point cloud
            impl_->settings_.model_.UnsetCustomDefaultColor();
            if (loaded_model)
            {
                impl_->settings_.model_.SetCurrentMaterials(
                    GuiSettingsModel::MATERIAL_FROM_FILE_NAME);
                impl_->settings_.view_->ShowFileMaterialEntry(true);
            }
            else
            {
                impl_->settings_.view_->ShowFileMaterialEntry(false);
            }
            impl_->settings_.view_->Update(); // make sure prefab material is correct

            auto &bounds = scene3d->GetBoundingBox();
            impl_->scene_wgt_->SetupCamera(60.0, bounds,
                                           bounds.GetCenter().cast<float>());

            // Make sure scene is redrawn
            impl_->scene_wgt_->ForceRedraw();
        }

        void WindowO3D::Layout(const gui::Theme &theme)
        {
            auto r = GetContentRect();
            const auto em = theme.font_size;
            impl_->scene_wgt_->SetFrame(r);

            // Draw help keys HUD in upper left
            // const auto pref = impl_->help_keys_->CalcPreferredSize(theme);
            // impl_->help_keys_->SetFrame(gui::Rect(0, r.y, pref.width, pref.height));
            // impl_->help_keys_->Layout(theme);

            // Draw camera HUD in lower left
            // const auto prefcam = impl_->help_camera_->CalcPreferredSize(theme);
            // impl_->help_camera_->SetFrame(gui::Rect(0, r.height + r.y - prefcam.height,
            //                                         prefcam.width, prefcam.height));
            // impl_->help_camera_->Layout(theme);

            // Settings in upper right
            const auto LIGHT_SETTINGS_WIDTH = 18 * em;
            auto light_settings_size =
                impl_->settings_.wgt_base->CalcPreferredSize(theme);
            gui::Rect lightSettingsRect(r.width - LIGHT_SETTINGS_WIDTH, r.y,
                                        LIGHT_SETTINGS_WIDTH,
                                        std::min(r.height, light_settings_size.height));
            impl_->settings_.wgt_base->SetFrame(lightSettingsRect);

            Super::Layout(theme);
        }

        void WindowO3D::LoadGeometry(const std::string &path)
        {
            auto progressbar = std::make_shared<gui::ProgressBar>();
            gui::Application::GetInstance().PostToMainThread(this, [this, path,
                                                                    progressbar]() {
                auto &theme = GetTheme();
                auto loading_dlg = std::make_shared<gui::Dialog>("Loading");
                auto vert =
                    std::make_shared<gui::Vert>(0, gui::Margins(theme.font_size));
                auto loading_text = std::string("Loading ") + path;
                vert->AddChild(std::make_shared<gui::Label>(loading_text.c_str()));
                vert->AddFixed(theme.font_size);
                vert->AddChild(progressbar);
                loading_dlg->AddChild(vert);
                ShowDialog(loading_dlg);
            });

            gui::Application::GetInstance().RunInThread([this, path, progressbar]() {
                auto UpdateProgress = [this, progressbar](float value) {
                    gui::Application::GetInstance().PostToMainThread(
                        this,
                        [progressbar, value]() { progressbar->SetValue(value); });
                };

                // clear current model
                impl_->loaded_model_.meshes_.clear();
                impl_->loaded_model_.materials_.clear();

                auto geometry_type = io::ReadFileGeometryType(path);

                bool model_success = false;
                if (geometry_type & io::CONTAINS_TRIANGLES)
                {
                    try
                    {
                        model_success = io::ReadTriangleModel(
                            path, impl_->loaded_model_, false);
                    }
                    catch (...)
                    {
                        model_success = false;
                    }
                }
                if (!model_success)
                {
                    utility::LogInfo("{} appears to be a point cloud", path.c_str());
                }

                auto geometry = std::shared_ptr<geometry::Geometry3D>();
                if (!model_success)
                {
                    auto cloud = std::make_shared<geometry::PointCloud>();
                    bool success = false;
                    const float ioProgressAmount = 0.5f;
                    try
                    {
                        io::ReadPointCloudOption opt;
                        opt.update_progress = [ioProgressAmount,
                                               UpdateProgress](double percent) -> bool {
                            UpdateProgress(ioProgressAmount * float(percent / 100.0));
                            return true;
                        };
                        success = io::ReadPointCloud(path, *cloud, opt);
                    }
                    catch (...)
                    {
                        success = false;
                    }
                    if (success)
                    {
                        utility::LogInfo("Successfully read {}", path.c_str());
                        UpdateProgress(ioProgressAmount);
                        if (!cloud->HasNormals())
                        {
                            cloud->EstimateNormals();
                        }
                        UpdateProgress(0.666f);
                        cloud->NormalizeNormals();
                        UpdateProgress(0.75f);
                        geometry = cloud;
                    }
                    else
                    {
                        utility::LogWarning("Failed to read points {}", path.c_str());
                        cloud.reset();
                    }
                }

                if (model_success || geometry)
                {
                    gui::Application::GetInstance().PostToMainThread(
                        this, [this, model_success, geometry]() {
                            SetGeometry(geometry, model_success);
                            CloseDialog();
                        });
                }
                else
                {
                    gui::Application::GetInstance().PostToMainThread(this, [this,
                                                                            path]() {
                        CloseDialog();
                        auto msg = std::string("Could not load '") + path + "'.";
                        ShowMessageBox("Error", msg.c_str());
                    });
                }
            });
        }

        void WindowO3D::ExportCurrentImage(const std::string &path)
        {
            impl_->scene_wgt_->EnableSceneCaching(false);
            impl_->scene_wgt_->GetScene()->GetScene()->RenderToImage(
                [this, path](std::shared_ptr<geometry::Image> image) mutable {
                    if (!io::WriteImage(path, *image))
                    {
                        this->ShowMessageBox(
                            "Error", (std::string("Could not write image to ") +
                                      path + ".")
                                         .c_str());
                    }
                    impl_->scene_wgt_->EnableSceneCaching(true);
                });
        }

        void WindowO3D::OnMenuItemSelected(gui::Menu::ItemId item_id)
        {
            auto menu_id = MenuId(item_id);
            switch (menu_id)
            {
            case FILE_OPEN:
            {
                auto dlg = std::make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::OPEN, "Open Geometry", GetTheme());
                dlg->AddFilter(".ply .stl .fbx .obj .off .gltf .glb",
                               "Triangle mesh files (.ply, .stl, .fbx, .obj, .off, "
                               ".gltf, .glb)");
                dlg->AddFilter(".xyz .xyzn .xyzrgb .ply .pcd .pts",
                               "Point cloud files (.xyz, .xyzn, .xyzrgb, .ply, "
                               ".pcd, .pts)");
                dlg->AddFilter(".ply", "Polygon files (.ply)");
                dlg->AddFilter(".stl", "Stereolithography files (.stl)");
                dlg->AddFilter(".fbx", "Autodesk Filmbox files (.fbx)");
                dlg->AddFilter(".obj", "Wavefront OBJ files (.obj)");
                dlg->AddFilter(".off", "Object file format (.off)");
                dlg->AddFilter(".gltf", "OpenGL transfer files (.gltf)");
                dlg->AddFilter(".glb", "OpenGL binary transfer files (.glb)");
                dlg->AddFilter(".xyz", "ASCII point cloud files (.xyz)");
                dlg->AddFilter(".xyzn", "ASCII point cloud with normals (.xyzn)");
                dlg->AddFilter(".xyzrgb",
                               "ASCII point cloud files with colors (.xyzrgb)");
                dlg->AddFilter(".pcd", "Point Cloud Data files (.pcd)");
                dlg->AddFilter(".pts", "3D Points files (.pts)");
                dlg->AddFilter("", "All files");
                dlg->SetOnCancel([this]() { this->CloseDialog(); });
                dlg->SetOnDone([this](const char *path) {
                    this->CloseDialog();
                    OnDragDropped(path);
                });
                ShowDialog(dlg);
                break;
            }
            case FILE_EXPORT_RGB:
            {
                auto dlg = std::make_shared<gui::FileDialog>(
                    gui::FileDialog::Mode::SAVE, "Save File", GetTheme());
                dlg->AddFilter(".png", "PNG images (.png)");
                dlg->AddFilter("", "All files");
                dlg->SetOnCancel([this]() { this->CloseDialog(); });
                dlg->SetOnDone([this](const char *path) {
                    this->CloseDialog();
                    this->ExportCurrentImage(path);
                });
                ShowDialog(dlg);
                break;
            }
            case FILE_QUIT:
                gui::Application::GetInstance().Quit();
                break;
            case SETTINGS_LIGHT_AND_MATERIALS:
            {
                auto visibility = !impl_->settings_.wgt_base->IsVisible();
                impl_->settings_.wgt_base->SetVisible(visibility);
                auto menubar = gui::Application::GetInstance().GetMenubar();
                menubar->SetChecked(SETTINGS_LIGHT_AND_MATERIALS, visibility);

                // We need relayout because materials settings pos depends on light
                // settings visibility
                Layout(GetTheme());

                break;
            }
            case HELP_KEYS:
            {
                bool is_visible = !impl_->help_keys_->IsVisible();
                impl_->help_keys_->SetVisible(is_visible);
                auto menubar = gui::Application::GetInstance().GetMenubar();
                menubar->SetChecked(HELP_KEYS, is_visible);
                break;
            }
            case HELP_CAMERA:
            {
                bool is_visible = !impl_->help_camera_->IsVisible();
                impl_->help_camera_->SetVisible(is_visible);
                auto menubar = gui::Application::GetInstance().GetMenubar();
                menubar->SetChecked(HELP_CAMERA, is_visible);
                if (is_visible)
                {
                    impl_->scene_wgt_->SetOnCameraChanged([this](rendering::Camera
                                                                     *cam) {
                        auto children = this->impl_->help_camera_->GetChildren();
                        auto set_text = [](const Eigen::Vector3f &v,
                                           std::shared_ptr<gui::Widget> label) {
                            auto l = std::dynamic_pointer_cast<gui::Label>(label);
                            l->SetText(fmt::format("[{:.2f} {:.2f} "
                                                   "{:.2f}]",
                                                   v.x(), v.y(), v.z())
                                           .c_str());
                        };
                        set_text(cam->GetPosition(), children[1]);
                        set_text(cam->GetForwardVector(), children[3]);
                        set_text(cam->GetLeftVector(), children[5]);
                        set_text(cam->GetUpVector(), children[7]);
                        this->SetNeedsLayout();
                    });
                }
                else
                {
                    impl_->scene_wgt_->SetOnCameraChanged(
                        std::function<void(rendering::Camera *)>());
                }
                break;
            }
            case HELP_ABOUT:
            {
                //                auto dlg = CreateAboutDialog(this);
                //                ShowDialog(dlg);
                break;
            }
            case HELP_CONTACT:
            {
                //                auto dlg = CreateContactDialog(this);
                //                ShowDialog(dlg);
                break;
            }
            case HELP_DEBUG:
            {
                break;
            }
            }
        }

        void WindowO3D::updateGeometry(std::shared_ptr<const geometry::PointCloud> sPC)
        {
            t::geometry::PointCloud tpcd = open3d::t::geometry::PointCloud::FromLegacyPointCloud(*sPC.get(), core::Dtype::Float32);
            gui::Application::GetInstance().PostToMainThread(
                this, [this, tpcd]() {
                    impl_->scene_wgt_->GetScene()->GetScene()->UpdateGeometry(MODEL_NAME,
                                                                              tpcd,
                                                                              open3d::visualization::rendering::Scene::kUpdatePointsFlag | open3d::visualization::rendering::Scene::kUpdateColorsFlag);
                    impl_->scene_wgt_->ForceRedraw();
                });
        }

    } // namespace visualization
} // namespace open3d

#endif
