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

            struct PCscanUI::Impl
            {
                vector<DrawObject> m_vObject;
                shared_ptr<O3DVisualizerSelections> m_sVertex;

                UIState m_uiState;
                Window *m_pWindow = nullptr;
                SceneWidget *m_pScene = nullptr;
                bool m_bScanning;
                string m_modelName;
                string m_areaName;
                Vector3d m_areaLineCol;
                vector<shared_ptr<Label3D>> m_vspDistLabel;

                //UI components
                Vert *m_panelCtrl;

                Button *m_btnScanStart;
                ProgressBar *m_progScan;
                Label *m_labelProg;

                Button *m_btnNewVertexSet;
                Button *m_btnDeleteVertexSet;
                ListView *m_listVertexSet;
                Label *m_labelArea;

                Slider *m_sliderPointSize;
                //UI components

                //UI handler
                O3D_UI_Cb m_cbBtnScan;
                O3D_UI_Cb m_cbBtnSavePC;
                O3D_UI_Cb m_cbBtnCamReset;

                void Construct(PCscanUI *w)
                {
                    if (m_pWindow)
                        return;

                    m_bScanning = false;
                    m_modelName = "PCMODEL";
                    m_areaName = "PCAREA";
                    m_areaLineCol = Vector3d(1.0, 0.0, 1.0);

                    m_cbBtnScan.init();
                    m_cbBtnSavePC.init();
                    m_cbBtnCamReset.init();

                    m_pWindow = w;
                    m_pScene = new SceneWidget();
                    m_sVertex = make_shared<O3DVisualizerSelections>(*m_pScene);
                    m_pScene->SetScene(make_shared<Open3DScene>(w->GetRenderer()));
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
                    w->AddChild(GiveOwnership(m_pScene));

                    InitCtrlPanel();
                    UpdateUIsettings();
                    SetMouseCameraMode();
                }

                void UpdateUIsettings(void)
                {
                    auto o3dscene = m_pScene->GetScene();
                    o3dscene->SetBackground(m_uiState.bg_color);
                    Eigen::Vector3f sun_dir(0, 0, 0);
                    o3dscene->SetLighting(Open3DScene::LightingProfile::NO_SHADOWS, sun_dir);

                    m_pScene->EnableSceneCaching(m_uiState.m_bSceneCache); // smoother UI with large geometry
                    ShowAxes(true);
                    SetPointSize(m_uiState.m_pointSize); // sync selections_' point size
                    SetLineWidth(m_uiState.m_lineWidth);
                }

                void InitCtrlPanel(void)
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

                    auto h = new Horiz(v_spacing);
                    auto btnCamSavePC = new Button("  Export .PLY  ");
                    btnCamSavePC->SetOnClicked([this]() {
                        OnExportPLY();
                    });
                    h->AddChild(GiveOwnership(btnCamSavePC));
                    h->AddStretch();

                    auto btnCamSaveRGB = new Button("  Export RGB  ");
                    btnCamSaveRGB->SetOnClicked([this]() {
                        OnExportRGB();
                    });
                    h->AddChild(GiveOwnership(btnCamSaveRGB));
                    h->AddStretch();
                    panelFile->AddChild(GiveOwnership(h));
                    panelFile->SetIsOpen(false);

                    // Camera
                    auto panelCam = new CollapsableVert("CAMERA", v_spacing, margins);
                    m_panelCtrl->AddChild(GiveOwnership(panelCam));
                    h = new Horiz(v_spacing);

                    auto btnCamAuto = new Button("     Auto     ");
                    btnCamAuto->SetOnClicked([this]() {
                        Vector3f CoR(0, 0, 0);
                        CamAutoBound(CoR);
                        SetMouseCameraMode();
                    });
                    h->AddChild(GiveOwnership(btnCamAuto));
                    h->AddStretch();

                    auto btnCamReset = new Button("     Reset    ");
                    btnCamReset->SetOnClicked([this]() {
                        m_cbBtnCamReset.call();
                        m_pScene->ForceRedraw();
                        SetMouseCameraMode();
                    });
                    h->AddChild(GiveOwnership(btnCamReset));
                    h->AddStretch();
                    panelCam->AddChild(GiveOwnership(h));

                    // Scan
                    auto panelScan = new CollapsableVert("SCAN", v_spacing, margins);
                    m_panelCtrl->AddChild(GiveOwnership(panelScan));

                    m_btnScanStart = new Button("     Start     ");
                    m_btnScanStart->SetToggleable(true);
                    m_btnScanStart->SetOnClicked([this]() {
                        SetMouseCameraMode();
                        RemoveAllVertexSet();

                        m_bScanning = !m_bScanning;

                        m_btnScanStart->SetOn(m_bScanning);
                        if (m_bScanning)
                        {
                            m_btnScanStart->SetText("     Stop     ");
                            m_btnNewVertexSet->SetEnabled(false);
                            m_btnDeleteVertexSet->SetEnabled(false);
                            m_listVertexSet->SetEnabled(false);
                        }
                        else
                        {
                            m_btnScanStart->SetText("     Start     ");
                            m_btnNewVertexSet->SetEnabled(true);
                            m_btnDeleteVertexSet->SetEnabled(true);
                            m_listVertexSet->SetEnabled(true);
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
                    h->AddChild(make_shared<Label>("(Ctrl-click to select a point)"));
                    h->AddStretch();
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

                    // Scene controls
                    auto panelSetting = new CollapsableVert("SETTING", v_spacing, margins);
                    panelSetting->SetIsOpen(false);
                    m_panelCtrl->AddChild(GiveOwnership(panelSetting));
                    m_sliderPointSize = new Slider(Slider::INT);
                    m_sliderPointSize->SetLimits(1, 10);
                    m_sliderPointSize->SetValue(m_uiState.m_pointSize);
                    m_sliderPointSize->SetOnValueChanged([this](const double newValue) {
                        this->SetPointSize(int(newValue));
                    });

                    auto *grid = new VGrid(2, v_spacing);
                    panelSetting->AddChild(GiveOwnership(grid));
                    grid->AddChild(make_shared<Label>("PointSize"));
                    grid->AddChild(GiveOwnership(m_sliderPointSize));
                }

                void SetCbBtnScan(OnBtnClickedCb pCb, void *pPCV)
                {
                    m_cbBtnScan.add(pCb, pPCV);
                }

                void SetCbBtnSavePC(OnBtnClickedCb pCb, void *pPCV)
                {
                    m_cbBtnSavePC.add(pCb, pPCV);
                }

                void SetCbBtnCamReset(OnBtnClickedCb pCb, void *pPCV)
                {
                    m_cbBtnCamReset.add(pCb, pPCV);
                }

                void SetProgressBar(float v)
                {
                    m_progScan->SetValue(v);
                    string s = "Memory used: " + i2str((int)(v * 100)) + "%";
                    m_labelProg->SetText(s.c_str());
                }

                void SetLabelArea(const string &s)
                {
                    m_labelArea->SetText(s.c_str());
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

                    mat.base_color = {1.0f, 1.0f, 1.0f, 1.0f};
                    mat.shader = kShaderDepth;
                    mat.point_size = ConvertToScaledPixels(m_uiState.m_pointSize);

                    m_vObject.push_back({name, nullptr, tgeom, mat, bVisible});
                    auto scene = m_pScene->GetScene();

                    scene->AddGeometry(name, tgeom.get(), mat);
                    scene->GetScene()->GeometryShadows(name, false, false);
                    scene->ShowGeometry(name, bVisible);

                    SetPointSize(m_uiState.m_pointSize);
                    m_pScene->ForceRedraw();
                }

                void UpdateTgeometry(const string &name, shared_ptr<t::geometry::PointCloud> tgeom)
                {
                    for (size_t i = 0; i < m_vObject.size(); i++)
                    {
                        DrawObject *pO = &m_vObject[i];
                        if (pO->m_name != name)
                            continue;

                        m_vObject[i].m_sTgeometry = tgeom;
                        break;
                    }
                }

                void RemoveGeometry(const string &name)
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

                DrawObject GetGeometry(const string &name) const
                {
                    for (auto &o : m_vObject)
                    {
                        if (o.m_name == name)
                            return o;
                    }
                    return DrawObject();
                }

                void CamSetProj(
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

                void CamSetPose(
                    const Eigen::Vector3f &center,
                    const Eigen::Vector3f &eye,
                    const Eigen::Vector3f &up)
                {
                    auto sCam = m_pScene->GetScene()->GetCamera();
                    sCam->LookAt(center, eye, up);

                    m_pScene->ForceRedraw();
                }

                void CamAutoBound(const Eigen::Vector3f &CoR)
                {
                    auto scene = m_pScene->GetScene();
                    auto sCam = scene->GetCamera();
                    m_pScene->SetupCamera(sCam->GetFieldOfView(),
                                          scene->GetBoundingBox(),
                                          CoR);
                    m_pScene->ForceRedraw();
                }

                void SetMouseCameraMode(void)
                {
                    if (m_sVertex->IsActive() && m_sVertex->GetNumberOfSets() > 0)
                        m_sVertex->MakeInactive();

                    m_pScene->SetViewControls(m_uiState.m_mouseMode);
                }

                void SetMousePickingMode(void)
                {
                    UpdateSelectableGeometry();
                    m_sVertex->MakeActive();
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
                        UpdateMaterial(o.m_name, o.m_material);
                    }

                    if (m_sVertex->GetNumberOfSets())
                        m_sVertex->SetPointSize(m_uiState.m_selectPointSize);

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
                        UpdateMaterial(o.m_name, o.m_material);
                    }
                    m_pScene->ForceRedraw();
                }

                void UpdateMaterial(const string &name,
                                    const Material &mat)
                {
                    m_pScene->GetScene()->GetScene()->OverrideMaterial(name, mat);
                }

                float ConvertToScaledPixels(int px)
                {
                    return round(px * m_pWindow->GetScaling());
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
                    UpdateArea();
                }

                void RemoveVertexSet(int index)
                {
                    if (index < 0)
                        return;
                    m_sVertex->RemoveSet(index);
                    UpdateVertexSetList();
                    UpdateArea();
                }

                void RemoveAllVertexSet(void)
                {
                    while (m_sVertex->GetNumberOfSets() > 0)
                        m_sVertex->RemoveSet(0);

                    UpdateVertexSetList();
                    UpdateArea();
                }

                void SelectVertexSet(int index)
                {
                    m_listVertexSet->SetSelectedIndex(index);
                    m_sVertex->SelectSet(index);
                }

                void UpdateVertexSetList(void)
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

                void UpdateArea(void)
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
                        spLS->colors_.push_back(m_areaLineCol);
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

                void RemoveDistLabel(void)
                {
                    for (shared_ptr<Label3D> spLabel : m_vspDistLabel)
                    {
                        m_pScene->RemoveLabel(spLabel);
                    }
                    m_vspDistLabel.clear();
                }

                double Area(vector<Vector3d> &vP)
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
                            m_pScene->EnableSceneCaching(m_uiState.m_bSceneCache);
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

                void OnExportPLY()
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
                                         shared_ptr<t::geometry::PointCloud> sTg,
                                         rendering::Material *material /*= nullptr*/,
                                         bool bVisible /*= true*/)
            {
                impl_->AddTgeometry(name, sTg, bVisible);
            }

            void PCscanUI::UpdatePointCloud(const string &name,
                                            shared_ptr<t::geometry::PointCloud> sTg)
            {
                impl_->UpdateTgeometry(name, sTg);

                gui::Application::GetInstance().PostToMainThread(
                    this, [this, name]() {
                        impl_->m_pScene->GetScene()->GetScene()->UpdateGeometry(
                            name,
                            *impl_->GetGeometry(name).m_sTgeometry,
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

            void PCscanUI::SetProgressBar(float v)
            {
                impl_->SetProgressBar(v);
            }

            vector<O3DVisualizerSelections::SelectionSet>
            PCscanUI::GetSelectionSets() const
            {
                return impl_->GetSelectionSets();
            }

            void PCscanUI::CamSetProj(
                double fov,
                double aspect,
                double near,
                double far,
                uint8_t fov_type)
            {
                impl_->CamSetProj(
                    fov,
                    aspect,
                    near,
                    far,
                    fov_type);
            }

            void PCscanUI::CamSetPose(
                const Eigen::Vector3f &center,
                const Eigen::Vector3f &eye,
                const Eigen::Vector3f &up)
            {
                impl_->CamSetPose(center, eye, up);
            }

            void PCscanUI::CamAutoBound(const Eigen::Vector3f &CoR)
            {
                impl_->CamAutoBound(CoR);
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

            void PCscanUI::SetCbBtnScan(OnBtnClickedCb pCb, void *pPCV)
            {
                impl_->SetCbBtnScan(pCb, pPCV);
            }

            void PCscanUI::SetCbBtnSavePC(OnBtnClickedCb pCb, void *pPCV)
            {
                impl_->SetCbBtnSavePC(pCb, pPCV);
            }

            void PCscanUI::SetCbBtnCamReset(OnBtnClickedCb pCb, void *pPCV)
            {
                impl_->SetCbBtnCamReset(pCb, pPCV);
            }

            UIState *PCscanUI::getUIState(void)
            {
                return &impl_->m_uiState;
            }

            void PCscanUI::UpdateUIsettings(void)
            {
                impl_->UpdateUIsettings();
            }

            void PCscanUI::SetMouseMode(void)
            {
                impl_->SetMouseCameraMode();
            }

        } // namespace visualizer
    }     // namespace visualization
} // namespace open3d

#endif