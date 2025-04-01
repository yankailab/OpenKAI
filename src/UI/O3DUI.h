#ifndef OpenKAI_src_UI_O3DUI_H_
#define OpenKAI_src_UI_O3DUI_H_

#include "../Base/open3d.h"
#include "../Utility/util.h"
#include "../Utility/utilStr.h"
#include <set>
#include <unordered_map>
#include <unordered_set>

using namespace open3d::geometry;
using namespace open3d::visualization::visualizer;
using namespace open3d::visualization::gui;
using namespace open3d::visualization::rendering;
using namespace std;
using namespace Eigen;
using namespace kai;

typedef void (*OnCbO3DUI)(void *pPCV, void *pD);
struct O3D_UI_Cb
{
	OnCbO3DUI m_pCb = NULL;
	void *m_pPCV = NULL;

	void add(OnCbO3DUI pCb, void *pPCV)
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
			enum UImode
			{
				uiMode_cam = 0,
				uiMode_pointPick = 1,
			};

			struct UIState
			{
				// general
				bool m_bSceneCache = false;
				bool m_bShowPanel = true;
				bool m_bShowAxes = true;
				string m_dirSave = "";

				// render
				int m_wPanel = 15;
				int m_sPoint = 2;
				int m_wLine = 1;
				float m_btnPaddingV = 0.5;
				float m_btnPaddingH = 0.5;
				Vector4f m_vBgCol = {0.0f, 0.0f, 0.0f, 0.0f};
				Vector3f m_vSunDir = {0.0f, 0.0f, 0.0f};
				Vector3d m_vAreaLineCol = {1.0, 0.0, 1.0};

				// filter
				double m_sVoxel = 0.01;
				int m_oRemovN = 10;
				double m_oRemovD = 0.1;

				// interactive
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

				// point cloud
				virtual void AddPointCloud(const string &name,
										   t::geometry::PointCloud* pTpc,
										   rendering::Material *pMaterial,
										   bool bVisible = true);
				virtual void UpdatePointCloud(const string &name,
											  t::geometry::PointCloud *pTpc);

				// mesh
				virtual void AddMesh(const string &name,
									 t::geometry::TriangleMesh *pTmesh,
									 rendering::Material *pMaterial,
									 bool bVisible = true);
				virtual void UpdateMesh(const string &name,
										t::geometry::TriangleMesh *pTmesh,
										rendering::Material *pMaterial);

				// line set
				virtual void AddLineSet(const string &name,
										geometry::LineSet *pLS,
										rendering::Material *pMaterial,
										bool bVisible = true);
				virtual void UpdateLineSet(const string &name,
										   geometry::LineSet *pLS,
										   rendering::Material *pMaterial);

				virtual void RemoveGeometry(const string &name);

				virtual void CamSetProj(
					double fov,
					double near,
					double far,
					uint8_t fov_type);

				virtual void CamSetProj(
					Camera::Projection projType,
					double left,
					double right,
					double bottom,
					double top,
					double near,
					double far);

				virtual void CamSetPose(
					const Vector3f &center,
					const Vector3f &eye,
					const Vector3f &up);

				virtual void CamAutoBound(const geometry::AxisAlignedBoundingBox &aabb,
										  const Vector3f &CoR);

				virtual void camMove(Vector3f vM);

				virtual UIState *getUIState(void);
				virtual void UpdateUIstate(void);
				virtual void SetPointSize(const string &name, int px);
				virtual void SetLineWidth(const string &name, int px);

				virtual void ShowMsg(const char *pTitle, const char *pMsg, bool bOK = false);
				virtual void CloseMsg(void);
				virtual void ExportCurrentImage(const string &path);
				virtual string getBaseDirSave(void);

			protected:
				virtual void Layout(const gui::LayoutContext &context);
				virtual float ConvertToScaledPixels(int px);

			protected:
				SceneWidget *m_pScene = NULL;
				UIState m_uiState;
			};

		} // namespace visualizer
	} // namespace visualization
} // namespace open3d

#endif
