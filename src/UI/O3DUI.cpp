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

            void O3DUI::AddPointCloud(const string &name,
                                      t::geometry::PointCloud *pTpc,
                                      rendering::Material *pMaterial,
                                      bool bVisible)
            {
                Material mat;
                if (pMaterial)
                {
                    mat = *pMaterial;
                }
                else
                {
                    mat.SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f});
                    mat.SetMaterialName("defaultUnlit");
                    mat.SetPointSize(ConvertToScaledPixels(m_uiState.m_sPoint));
                }
                MaterialRecord mr;
                mat.ToMaterialRecord(mr);

                auto scene = m_pScene->GetScene();
                scene->AddGeometry(name, pTpc, mr, false);
                scene->GetScene()->GeometryShadows(name, false, false);
                scene->ShowGeometry(name, bVisible);

                // SetPointSize(m_uiState.m_sPoint);
                // SetLineWidth(m_uiState.m_wLine);
                m_pScene->ForceRedraw();
            }

            void O3DUI::UpdatePointCloud(const string &name,
                                         t::geometry::PointCloud *pTpc)
            {
                gui::Application::GetInstance().PostToMainThread(
                    this, [this, name, pTpc]()
                    {
                        m_pScene->GetScene()->GetScene()->UpdateGeometry(
                            name,
                            *pTpc,
                            open3d::visualization::rendering::Scene::kUpdatePointsFlag |
                                open3d::visualization::rendering::Scene::kUpdateColorsFlag);

                        m_pScene->ForceRedraw(); });
            }

            void O3DUI::AddMesh(const string &name,
                                t::geometry::TriangleMesh *pTmesh,
                                rendering::Material *pMaterial,
                                bool bVisible)
            {
                Material mat;
                if (pMaterial)
                {
                    mat = *pMaterial;
                }
                else
                {
                    mat.SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f});
                    mat.SetMaterialName("defaultUnlit");
                    mat.SetPointSize(ConvertToScaledPixels(m_uiState.m_sPoint));
                }
                MaterialRecord mr;
                mat.ToMaterialRecord(mr);

                auto scene = m_pScene->GetScene();
                scene->AddGeometry(name, pTmesh, mr, false);
                scene->GetScene()->GeometryShadows(name, false, false);
                scene->ShowGeometry(name, bVisible);

                // SetPointSize(m_uiState.m_sPoint);
                // SetLineWidth(m_uiState.m_wLine);
                m_pScene->ForceRedraw();
            }

            void O3DUI::UpdateMesh(const string &name,
                                   t::geometry::TriangleMesh *pTmesh)
            {
                // gui::Application::GetInstance().PostToMainThread(
                //     this, [this, name, pTmesh]()
                //     {
                //         m_pScene->GetScene()->GetScene()->UpdateGeometry(
                //             name,
                //             *pTmesh,
                //             open3d::visualization::rendering::Scene::kUpdatePointsFlag |
                //                 open3d::visualization::rendering::Scene::kUpdateColorsFlag);

                //         m_pScene->ForceRedraw();
                //     });
            }

            void O3DUI::AddLineSet(const string &name,
                                   geometry::LineSet *pG,
                                   rendering::Material *pMaterial,
                                   bool bVisible)
            {
                Material mat;
                if (pMaterial)
                {
                    mat = *pMaterial;
                }
                else
                {
                    mat.SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f});
                    mat.SetMaterialName("unlitLine");
                    mat.SetLineWidth(m_uiState.m_wLine * GetScaling());
                }
                MaterialRecord mr;
                mat.ToMaterialRecord(mr);

                auto scene = m_pScene->GetScene();
                scene->AddGeometry(name, pG, mr);
                scene->GetScene()->GeometryShadows(name, false, false);
                scene->ShowGeometry(name, bVisible);

                m_pScene->ForceRedraw();
            }

            void O3DUI::UpdateLineSet(const string &name,
                                      geometry::LineSet *pG)
            {
                gui::Application::GetInstance().PostToMainThread(
                    this, [this, name, pG]()
                    {
                        auto pS = m_pScene->GetScene();
                        pS->GetScene()->RemoveGeometry(name);

                        Material mat;
                        mat.SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f});
                        mat.SetMaterialName("unlitLine");
                        mat.SetLineWidth(10);

                        MaterialRecord mr;
                        mat.ToMaterialRecord(mr);

                        pS->AddGeometry(name, pG, mr);
                        pS->GetScene()->GeometryShadows(name, false, false);
                        pS->ShowGeometry(name, true);

                        m_pScene->ForceRedraw(); });
            }

            void O3DUI::RemoveGeometry(const string &name)
            {
                m_pScene->GetScene()->RemoveGeometry(name);
                m_pScene->ForceRedraw();
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
                // SetPointSize(m_uiState.m_sPoint);
                // SetLineWidth(m_uiState.m_wLine);

                SetNeedsLayout();
                m_pScene->ForceRedraw();
            }

            void O3DUI::SetPointSize(const string &name, int px)
            {
                m_uiState.m_sPoint = px;
                px = int(ConvertToScaledPixels(px));
                // for (auto &o : m_vObject)
                // {
                //     o.m_material.SetPointSize(float(px));
                //     MaterialRecord mr;
                //     o.m_material.ToMaterialRecord(mr);
                //     m_pScene->GetScene()->GetScene()->OverrideMaterial(o.m_name, mr);
                // }
                m_pScene->SetPickablePointSize(px);

                m_pScene->ForceRedraw();
            }

            void O3DUI::SetLineWidth(const string &name, int px)
            {
                m_uiState.m_wLine = px;

                px = int(ConvertToScaledPixels(px));
                // for (auto &o : m_vObject)
                // {
                //     o.m_material.SetLineWidth(float(px));
                //     MaterialRecord mr;
                //     o.m_material.ToMaterialRecord(mr);
                //     m_pScene->GetScene()->GetScene()->OverrideMaterial(o.m_name, mr);
                // }
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
                if (!pDir)
                    return "";

                struct dirent *dir;
                string d = "";
                while ((dir = readdir(pDir)) != NULL)
                {
                    IF_CONT(dir->d_name[0] == '.');
                    IF_CONT(dir->d_type != 0x4); // 0x4: folder

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

            float O3DUI::ConvertToScaledPixels(int px)
            {
                return round(px * GetScaling());
            }

        } // namespace visualizer
    } // namespace visualization
} // namespace open3d
