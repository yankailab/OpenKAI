#pragma once
#ifdef USE_OPEN3D
#include "../Base/open3d.h"
#include "../Utility/util.h"
#include "../Utility/utilStr.h"
#include <set>
#include <unordered_map>
#include <unordered_set>
using namespace open3d::visualization::gui;
using namespace open3d::visualization::rendering;
using namespace std;
using namespace Eigen;
using namespace kai;

typedef void (*OnBtnClickedCb)(void *pPCV, void *pD);
struct O3D_UI_Cb
{
	OnBtnClickedCb m_pCb = NULL;
	void *m_pPCV = NULL;

	void add(OnBtnClickedCb pCb, void *pPCV)
	{
		m_pCb = pCb;
		m_pPCV = pPCV;
	}

	bool bValid(void)
	{
		return (m_pCb && m_pPCV) ? true : false;
	}

	void call(void *pD = NULL)
	{
		if (!bValid())
			return;

		m_pCb(m_pPCV, pD);
	}
};

namespace open3d
{

	namespace geometry
	{
		class Geometry3D;
		class Image;
	}

	namespace t
	{
		namespace geometry
		{
			class Geometry;
		}
	}

	namespace visualization
	{

		namespace rendering
		{
			class Open3DScene;
		}

		namespace visualizer
		{
			struct DrawObject
			{
				string m_name;
				shared_ptr<geometry::Geometry3D> m_sGeometry;
				shared_ptr<t::geometry::PointCloud> m_sTgeometry;
				rendering::Material m_material;
				bool m_bVisible = true;
			};

			enum UImode
			{
				uiMode_cam = 0,
				uiMode_pointPick = 1,
			};

			struct UIState
			{
				//general
				bool m_bSceneCache = false;
				bool m_bShowPanel = true;
				bool m_bShowAxes = true;

				//render
				int m_wPanel = 15;
				int m_sPoint = 2;
				int m_wLine = 15;
				Vector4f m_vBgCol = {0.0f, 0.0f, 0.0f, 0.0f};
				Vector3f m_vSunDir = {0.0f, 0.0f, 0.0f};
                Vector3d m_vAreaLineCol = {1.0, 0.0, 1.0};

				//filter
				double m_sVoxel = 0.01;
				int m_oRemovN = 10;
				double m_oRemovD = 0.1;

				//interactive
				Vector3f m_vCamPos;
				float m_sMove = 0.5;
				SceneWidget::Controls m_mouseMode = SceneWidget::Controls::ROTATE_CAMERA;
			};

			class O3DUI : public gui::Window
			{
			public:
				O3DUI(const string &title, int width, int height);
				virtual ~O3DUI();

				virtual void Init(void);

				virtual void AddGeometry(const string &name,
								 shared_ptr<geometry::Geometry3D> spG,
								 rendering::Material *material = nullptr,
								 bool bVisible = true);
				virtual void AddPointCloud(const string &name,
								   shared_ptr<t::geometry::PointCloud> sTg,
								   rendering::Material *material = nullptr,
								   bool bVisible = true);
				virtual void UpdatePointCloud(const string &name,
									  shared_ptr<t::geometry::PointCloud> sTg);
				virtual void RemoveGeometry(const string &name);
				virtual DrawObject GetGeometry(const string &name) const;

				virtual void CamSetProj(
					double fov,
					double near,
					double far,
					uint8_t fov_type);

				virtual void CamSetPose(
					const Vector3f &center,
					const Vector3f &eye,
					const Vector3f &up);

				virtual void CamAutoBound(const geometry::AxisAlignedBoundingBox& aabb,
                                  const Vector3f &CoR);

				virtual void camMove(Vector3f vM);

				virtual UIState *getUIState(void);
				virtual void UpdateUIstate(void);
            	virtual void SetPointSize(int px);
    	        virtual void SetLineWidth(int px);

				virtual void ShowMsg(const char* pTitle, const char* pMsg, bool bOK = false);
				virtual void CloseMsg(void);
				virtual void ExportCurrentImage(const string &path);

			protected:
				virtual void Layout(const gui::Theme &theme);
	            virtual void UpdateTgeometry(const string &name, shared_ptr<t::geometry::PointCloud> sTg);
	            virtual float ConvertToScaledPixels(int px);

			protected:
				vector<DrawObject> m_vObject;
				SceneWidget *m_pScene = nullptr;
				UIState m_uiState;
				string m_modelName;
			};

		} // namespace visualizer
	}	  // namespace visualization
} // namespace open3d

#endif
