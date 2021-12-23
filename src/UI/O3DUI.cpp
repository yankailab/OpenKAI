#include "O3DUI.h"

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

            O3DUI::O3DUI(const string &title, int width, int height)
                : Window(title, width, height)
            {
            }

            O3DUI::~O3DUI() {}

            void O3DUI::Init(void)
            {
                m_modelName = "PCMODEL";
                m_pScene = new SceneWidget();
                m_pScene->SetScene(make_shared<Open3DScene>(GetRenderer()));
                AddChild(GiveOwnership(m_pScene));
            }

            void O3DUI::AddGeometry(const string &name,
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
                        mat.line_width = m_uiState.m_wLine * GetScaling();
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

            void O3DUI::AddPointCloud(const string &name,
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

            void O3DUI::UpdatePointCloud(const string &name,
                                         shared_ptr<t::geometry::PointCloud> sTg)
            {
                UpdateTgeometry(name, sTg);

                gui::Application::GetInstance().PostToMainThread(
                    this, [this, name]()
                    {
                        m_pScene->GetScene()->GetScene()->UpdateGeometry(
                            name,
                            *GetGeometry(name).m_sTgeometry,
                            open3d::visualization::rendering::Scene::kUpdatePointsFlag |
                                open3d::visualization::rendering::Scene::kUpdateColorsFlag);

                        m_pScene->ForceRedraw();
                    });
            }

            void O3DUI::RemoveGeometry(const string &name)
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

            DrawObject O3DUI::GetGeometry(const string &name) const
            {
                for (auto &o : m_vObject)
                {
                    if (o.m_name == name)
                        return o;
                }
                return DrawObject();
            }

            void O3DUI::CamSetProj(
                double fov,
                double near,
                double far,
                uint8_t fov_type)
            {
                auto f = m_pScene->GetFrame();
                auto sCam = m_pScene->GetScene()->GetCamera();
                sCam->SetProjection(
                    fov,
                    float(f.width) / float(f.height),
                    near,
                    far,
                    (fov_type == 0) ? Camera::FovType::Horizontal : Camera::FovType::Vertical);
                m_pScene->ForceRedraw();
            }

            void O3DUI::CamSetPose(
                const Vector3f &center,
                const Vector3f &eye,
                const Vector3f &up)
            {
                auto sCam = m_pScene->GetScene()->GetCamera();
                sCam->LookAt(center, eye, up);
                m_pScene->ForceRedraw();
            }

            void O3DUI::CamAutoBound(const geometry::AxisAlignedBoundingBox &aabb,
                                     const Vector3f &CoR)
            {
                m_pScene->SetupCamera(m_pScene->GetScene()->GetCamera()->GetFieldOfView(),
                                      aabb,
                                      CoR);
                m_pScene->ForceRedraw();
            }

            void O3DUI::camMove(Vector3f vM)
            {
                auto pC = m_pScene->GetScene()->GetCamera();
                auto mm = pC->GetModelMatrix();
                vM *= m_uiState.m_sMove;
                mm.translate(vM);
                pC->SetModelMatrix(mm);
                m_pScene->ForceRedraw();
            }

            UIState *O3DUI::getUIState(void)
            {
                return &m_uiState;
            }

            void O3DUI::UpdateUIstate(void)
            {
                m_pScene->EnableSceneCaching(m_uiState.m_bSceneCache);
                auto pO3DScene = m_pScene->GetScene();
                pO3DScene->ShowAxes(m_uiState.m_bShowAxes);
                pO3DScene->SetBackground(m_uiState.m_vBgCol, nullptr);
                pO3DScene->SetLighting(Open3DScene::LightingProfile::NO_SHADOWS, m_uiState.m_vSunDir);
                SetPointSize(m_uiState.m_sPoint);
                SetLineWidth(m_uiState.m_wLine);

                SetNeedsLayout();
                m_pScene->ForceRedraw();
            }

            void O3DUI::SetPointSize(int px)
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

            void O3DUI::SetLineWidth(int px)
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

            void O3DUI::ShowMsg(const char *pTitle, const char *pMsg, bool bOK)
            {
                auto em = GetTheme().font_size;
                auto margins = Margins(GetTheme().default_margin);
                auto dlg = std::make_shared<Dialog>(pTitle);
                auto layout = std::make_shared<Vert>(em, margins);
                layout->AddChild(std::make_shared<Label>(pMsg));
                if (bOK)
                {
                    auto ok = std::make_shared<Button>("OK");
                    ok->SetOnClicked([this]()
                                     { this->CloseDialog(); });
                    layout->AddChild(Horiz::MakeCentered(ok));
                }

                dlg->AddChild(layout);
                ShowDialog(dlg);
                PostRedraw();
            }

            void O3DUI::CloseMsg(void)
            {
                CloseDialog();
                PostRedraw();
            }

            void O3DUI::ExportCurrentImage(const string &path)
            {
                m_pScene->EnableSceneCaching(false);
                m_pScene->GetScene()->GetScene()->RenderToImage(
                    [this, path](shared_ptr<geometry::Image> image) mutable
                    {
                        if (!io::WriteImage(path, *image))
                        {
                            ShowMessageBox(
                                "Error",
                                (string("Could not write image to ") + path + ".").c_str());
                        }
                        m_pScene->EnableSceneCaching(m_uiState.m_bSceneCache);
                    });
            }

            string O3DUI::getBaseDirSave(void)
            {
                DIR *pDir = opendir(m_uiState.m_dirSave.c_str());
                if(!pDir)return "";

                struct dirent *dir;
                string d = "";
                while ((dir = readdir(pDir)) != NULL)
                {
                    IF_CONT(dir->d_name[0] == '.');
                    IF_CONT(dir->d_type != 0x4); //0x4: folder

                    d = m_uiState.m_dirSave + string(dir->d_name);
                    d = checkDirName(d);
                    break;
                }

                closedir(pDir);
                return d;
            }

            void O3DUI::Layout(const gui::LayoutContext &context)
            {
                m_pScene->SetFrame(GetContentRect());
                gui::Window::Layout(context);
            }

            void O3DUI::UpdateTgeometry(const string &name, shared_ptr<t::geometry::PointCloud> sTg)
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

            float O3DUI::ConvertToScaledPixels(int px)
            {
                return round(px * GetScaling());
            }

        } // namespace visualizer
    }     // namespace visualization
} // namespace open3d
