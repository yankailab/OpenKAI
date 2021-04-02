#ifndef OpenKAI_src_Application_PointCloud_PCviewerUI_H_
#define OpenKAI_src_Application_PointCloud_PCviewerUI_H_
#ifdef USE_OPEN3D
#include "../Base/common.h"

namespace open3d
{

	namespace visualization
	{
		const std::string _MODEL_NAME = "__model__";
        enum MENU_ID
        {
            M_FILE_OPEN,
            M_FILE_EXPORT_RGB,
            M_FILE_QUIT,
        };
		
		namespace gui
		{
			struct Theme;
		}

		typedef void ( *OnBtnClickedCb ) ( void* pPCV );
		struct PCviewerUICb
		{
			OnBtnClickedCb m_pCb;
			void* m_pPCV;

			void init(void)
			{
				m_pCb = NULL;
				m_pPCV = NULL;
			}
		};

		class PCviewerUI : public GuiVisualizer
		{
			using Super = GuiVisualizer;

		public:
			PCviewerUI(const std::string &title, int width, int height);
			virtual ~PCviewerUI();

			void SetGeometry(std::shared_ptr<const geometry::Geometry> geometry);
			void UpdateGeometry(std::shared_ptr<const geometry::PointCloud> sPC);

			/// Loads asynchronously, will return immediately.
			void ExportCurrentImage(const std::string &path);
			void Layout(const gui::Theme &theme) override;

			void setDevice(const string& device);

		protected:
			void Init();
			virtual void OnMenuItemSelected(gui::Menu::ItemId item_id) override;

			struct Impl;
			std::unique_ptr<Impl> impl_;

			string m_strDevice;
		};

		struct PCviewerUI::Impl
		{
			std::shared_ptr<gui::SceneWidget> scene_wgt_;
			std::shared_ptr<gui::VGrid> help_keys_;
			std::shared_ptr<gui::VGrid> help_camera_;
			std::shared_ptr<Receiver> receiver_;

			struct Settings
			{
				rendering::Material lit_material_;
				rendering::Material unlit_material_;
				rendering::Material normal_depth_material_;

				GuiSettingsModel model_;
				std::shared_ptr<gui::Vert> wgt_base;
				std::shared_ptr<gui::Button> wgt_mouse_arcball;
				std::shared_ptr<gui::Button> wgt_mouse_fly;
				std::shared_ptr<gui::Button> wgt_mouse_model;
				std::shared_ptr<gui::Button> wgt_mouse_sun;
				std::shared_ptr<gui::Button> wgt_mouse_ibl;
				std::shared_ptr<GuiSettingsView> view_;
			} settings_;

			rendering::TriangleMeshModel loaded_model_;

			int app_menu_custom_items_index_ = -1;
			std::shared_ptr<gui::Menu> app_menu_;

			void InitializeMaterials(rendering::Renderer &renderer,
									 const std::string &resource_path)
			{
				settings_.lit_material_.shader = "defaultLit";
				settings_.unlit_material_.shader = "defaultUnlit";

				auto &defaults = settings_.model_.GetCurrentMaterials();

				UpdateMaterials(renderer, defaults);
			}

			void SetMaterialsToDefault()
			{
				settings_.view_->ShowFileMaterialEntry(false);

				settings_.model_.SetMaterialsToDefault();
				// model's OnChanged callback will get called (if set), which will
				// update everything.
			}

			bool SetIBL(rendering::Renderer &renderer, const std::string &path)
			{
				auto *render_scene = scene_wgt_->GetScene()->GetScene();
				std::string ibl_name(path);
				if (ibl_name.empty())
				{
					ibl_name =
						std::string(
							gui::Application::GetInstance().GetResourcePath()) +
						"/" + GuiSettingsModel::DEFAULT_IBL;
				}
				if (ibl_name.find("_ibl.ktx") != std::string::npos)
				{
					ibl_name = ibl_name.substr(0, ibl_name.size() - 8);
				}
				render_scene->SetIndirectLight(ibl_name);
				float intensity = render_scene->GetIndirectLightIntensity();
				render_scene->SetIndirectLightIntensity(intensity);
				scene_wgt_->ForceRedraw();

				return true;
			}

			void SetMouseControls(gui::Window &window,
								  gui::SceneWidget::Controls mode)
			{
				using Controls = gui::SceneWidget::Controls;
				scene_wgt_->SetViewControls(mode);
				window.SetFocusWidget(scene_wgt_.get());
				settings_.wgt_mouse_arcball->SetOn(mode == Controls::ROTATE_CAMERA);
				settings_.wgt_mouse_fly->SetOn(mode == Controls::FLY);
				settings_.wgt_mouse_model->SetOn(mode == Controls::ROTATE_MODEL);
				settings_.wgt_mouse_sun->SetOn(mode == Controls::ROTATE_SUN);
				settings_.wgt_mouse_ibl->SetOn(mode == Controls::ROTATE_IBL);
			}

			void UpdateFromModel(rendering::Renderer &renderer, bool material_changed)
			{
				Eigen::Vector3f bcolor = settings_.model_.GetBackgroundColor();
				bcolor.setZero();
				
				scene_wgt_->GetScene()->SetBackground(
					{bcolor.x(), bcolor.y(), bcolor.z(), 1.f});

				if (settings_.model_.GetShowSkybox())
				{
					scene_wgt_->GetScene()->ShowSkybox(true);
				}
				else
				{
					scene_wgt_->GetScene()->ShowSkybox(false);
				}
				scene_wgt_->ShowSkybox(settings_.model_.GetShowSkybox());

				scene_wgt_->GetScene()->ShowAxes(settings_.model_.GetShowAxes());

				UpdateLighting(renderer, settings_.model_.GetLighting());

				// Make sure scene redraws once changes have been applied
				scene_wgt_->ForceRedraw();

				// Bail early if there were no material property changes
				if (!material_changed)
					return;

				auto &current_materials = settings_.model_.GetCurrentMaterials();
				if (settings_.model_.GetMaterialType() ==
						GuiSettingsModel::MaterialType::LIT &&
					current_materials.lit_name ==
						GuiSettingsModel::MATERIAL_FROM_FILE_NAME)
				{
					scene_wgt_->GetScene()->UpdateModelMaterial(_MODEL_NAME,
																loaded_model_);
				}
				else
				{
					UpdateMaterials(renderer, current_materials);
					switch (settings_.model_.GetMaterialType())
					{
					case GuiSettingsModel::MaterialType::LIT:
						scene_wgt_->GetScene()->UpdateMaterial(
							settings_.lit_material_);
						break;
					case GuiSettingsModel::MaterialType::UNLIT:
						scene_wgt_->GetScene()->UpdateMaterial(
							settings_.unlit_material_);
						break;
					case GuiSettingsModel::MaterialType::NORMAL_MAP:
					{
						settings_.normal_depth_material_.shader = "normals";
						scene_wgt_->GetScene()->UpdateMaterial(
							settings_.normal_depth_material_);
					}
					break;
					case GuiSettingsModel::MaterialType::DEPTH:
					{
						settings_.normal_depth_material_.shader = "depth";
						scene_wgt_->GetScene()->UpdateMaterial(
							settings_.normal_depth_material_);
					}
					break;

					default:
						break;
					}
				}

				auto *view = scene_wgt_->GetRenderView();
				switch (settings_.model_.GetMaterialType())
				{
				case GuiSettingsModel::MaterialType::LIT:
				{
					view->SetMode(rendering::View::Mode::Color);
					break;
				}
				case GuiSettingsModel::MaterialType::UNLIT:
				{
					view->SetMode(rendering::View::Mode::Color);
					break;
				}
				case GuiSettingsModel::MaterialType::NORMAL_MAP:
					view->SetMode(rendering::View::Mode::Normals);
					break;
				case GuiSettingsModel::MaterialType::DEPTH:
					view->SetMode(rendering::View::Mode::Depth);
					break;
				}
			}

		private:
			void UpdateLighting(rendering::Renderer &renderer,
								const GuiSettingsModel::LightingProfile &lighting)
			{
				auto scene = scene_wgt_->GetScene();
				auto *render_scene = scene->GetScene();
				if (lighting.use_default_ibl)
				{
					this->SetIBL(renderer, "");
				}

				render_scene->EnableIndirectLight(lighting.ibl_enabled);
				render_scene->SetIndirectLightIntensity(float(lighting.ibl_intensity));
				render_scene->SetIndirectLightRotation(lighting.ibl_rotation);
				render_scene->SetSunLight(lighting.sun_dir, lighting.sun_color,
										  float(lighting.sun_intensity));
				render_scene->EnableSunLight(lighting.sun_enabled);
			}

			void UpdateMaterials(rendering::Renderer &renderer,
								 const GuiSettingsModel::Materials &materials)
			{
				auto &lit = settings_.lit_material_;
				auto &unlit = settings_.unlit_material_;
				auto &normal_depth = settings_.normal_depth_material_;

				// Update lit from GUI
				lit.base_color.x() = materials.lit.base_color.x();
				lit.base_color.y() = materials.lit.base_color.y();
				lit.base_color.z() = materials.lit.base_color.z();
				lit.point_size = materials.point_size;
				lit.base_metallic = materials.lit.metallic;
				lit.base_roughness = materials.lit.roughness;
				lit.base_reflectance = materials.lit.reflectance;
				lit.base_clearcoat = materials.lit.clear_coat;
				lit.base_clearcoat_roughness = materials.lit.clear_coat_roughness;
				lit.base_anisotropy = materials.lit.anisotropy;

				// Update unlit from GUI
				unlit.base_color.x() = materials.unlit.base_color.x();
				unlit.base_color.y() = materials.unlit.base_color.y();
				unlit.base_color.z() = materials.unlit.base_color.z();
				unlit.point_size = materials.point_size;

				// Update normal/depth from GUI
				normal_depth.point_size = materials.point_size;
			}

			void OnNewIBL(Window &window, const char *name)
			{
				std::string path = gui::Application::GetInstance().GetResourcePath();
				path += std::string("/") + name + "_ibl.ktx";
				if (!SetIBL(window.GetRenderer(), path))
				{
					// must be the "Custom..." option
					auto dlg = std::make_shared<gui::FileDialog>(
						gui::FileDialog::Mode::OPEN, "Open HDR Map",
						window.GetTheme());
					dlg->AddFilter(".ktx", "Khronos Texture (.ktx)");
					dlg->SetOnCancel([&window]() { window.CloseDialog(); });
					dlg->SetOnDone([this, &window](const char *path) {
						window.CloseDialog();
						SetIBL(window.GetRenderer(), path);
						// We need to set the "custom" bit, so just call the current
						// profile a custom profile.
						settings_.model_.SetCustomLighting(
							settings_.model_.GetLighting());
					});
					window.ShowDialog(dlg);
				}
			}
		};

	}
}

#endif
#endif
