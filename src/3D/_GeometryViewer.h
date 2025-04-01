/*
 * _GeometryViewer.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D__GeometryViewer_H_
#define OpenKAI_src_3D__GeometryViewer_H_

#include "PointCloud/_PCstream.h"
#include "PointCloud/_PCframe.h"
#include "PointCloud/_PCgrid.h"
#include "../UI/O3DUI.h"

namespace kai
{
	enum GVIEWER_THREAD_ACTION
	{
		gv_ScanReset,
		gv_ScanSet,
		gv_ScanStart,
		gv_ScanStop,
		gv_ScanTake,
		gv_SavePC,
		gv_Scanning,
		gv_VoxelDown,
		gv_HiddenRemove,
		gv_ResetPC,
		gv_CamAuto,
		gv_CamCtrl,
		gv_RefreshCol,
	};

	struct GVIEWER_CAM_PROJ
	{
		int m_type = 0; //Perspective, Ortho
		float m_fov = 70.0;
		uint8_t m_fovType = 0;

		vFloat2 m_vLR = {-10, 10};
		vFloat2 m_vBT = {-10, 10};
		vFloat2 m_vNF = {0, FLT_MAX}; // near & far plane
	};

	struct GVIEWER_CAM
	{
		vFloat3 m_vLookAt = {0, 0, 0};
		vFloat3 m_vEye = {0, 0, 1};
		vFloat3 m_vUp = {0, 1, 0};
	};

	struct GVIEWER_OBJ
	{
		_GeometryBase *m_pGB = NULL;
		string m_name;

		bool m_bStatic = true;
		int m_nPbuf = 0;
		float m_rDummyDome = 1000.0;
		PointCloud m_PC;
		t::geometry::PointCloud m_tPC;
		LineSet m_ls;
		int m_iGridLS = 0;

		Material m_mat;
		string m_matName;
		vFloat4 m_matCol = {1, 1, 1, 1};
		int m_matPointSize = 1;
		int m_matLineWidth = 1;

		void init(void)
		{
			IF_(m_nPbuf < 0);

			m_PC.Clear();
			m_PC.points_.reserve(m_nPbuf);
			m_PC.colors_.reserve(m_nPbuf);
		}

		void updateMaterial(void)
		{
			m_mat.SetMaterialName(m_matName);
			m_mat.SetLineWidth(m_matLineWidth);
			m_mat.SetPointSize(m_matPointSize);
			m_mat.SetBaseColor({m_matCol.x, m_matCol.y, m_matCol.z, m_matCol.w});
		}

		void updateGeometry(void)
		{
			GEOMETRY_TYPE gt = m_pGB->getType();

			if (gt == pc_stream)
			{
				addPCstream();
				adjustNpoints(&m_PC, m_PC.points_.size(), m_nPbuf);
				m_tPC = t::geometry::PointCloud::FromLegacy(m_PC, core::Dtype::Float32);
			}
			else if (gt == pc_frame)
			{
				_PCframe *pF = (_PCframe *)m_pGB;
				pF->copyTo(&m_PC);
				adjustNpoints(&m_PC, m_PC.points_.size(), m_nPbuf);
				m_tPC = t::geometry::PointCloud::FromLegacy(m_PC, core::Dtype::Float32);
			}
			else if (gt == pc_grid)
			{
				_PCgrid *p = (_PCgrid *)m_pGB;

				if (m_bStatic)
				{
					m_ls = *p->getGridLines();
				}
				else
				{
					p->getActiveCellLines(&m_ls, m_iGridLS);
				}
			}
		}

		void addPCstream(uint64_t tExpire = 0)
		{
			_PCstream *p = (_PCstream *)m_pGB;
			m_PC.Clear();

			uint64_t tNow = getApproxTbootUs();
			for (int i = 0; i < p->nP(); i++)
			{
				GEOMETRY_POINT *pP = p->get(i);
				if (tExpire)
				{
					IF_CONT(bExpired(pP->m_tStamp, tExpire, tNow));
				}

				m_PC.points_.push_back(v2e(pP->m_vP).cast<double>());
				m_PC.colors_.push_back(v2e(pP->m_vC).cast<double>());

				IF_(m_PC.points_.size() >= m_nPbuf);
			}
		}

		void adjustNpoints(PointCloud *pPC, int nP, int nPbuf)
		{
			NULL_(pPC);

			if (nP < nPbuf)
			{
				addDummyPoints(pPC, nPbuf - nP, m_rDummyDome);
			}
			else if (nP > nPbuf)
			{
				int d = nP - nPbuf;
				pPC->points_.erase(pPC->points_.end() - d, pPC->points_.end());
				pPC->colors_.erase(pPC->colors_.end() - d, pPC->colors_.end());
			}
		}

		void addDummyPoints(PointCloud *pPC, int n, float r, Vector3d vCol = {0, 0, 0})
		{
			NULL_(pPC);

			float nV = floor(sqrt((float)n));
			float nH = ceil(n / nV);

			float dV = OK_PI / nV;
			float dH = (OK_PI * 2.0) / nH;

			int k = 0;
			for (int i = 0; i < nH; i++)
			{
				float h = dH * i;
				float sinH = sin(h);
				float cosH = cos(h);

				for (int j = 0; j < nV; j++)
				{
					float v = dV * j;
					float sinV = sin(v);
					float cosV = cos(v);

					Vector3d vP(
						r * sinV * sinH,
						r * sinV * cosH,
						r * cosV);

					pPC->points_.push_back(vP);
					pPC->colors_.push_back(vCol);

					IF_(++k >= n);
				}
			}
		}
	};

	class _GeometryViewer : public _GeometryBase
	{
	public:
		_GeometryViewer();
		virtual ~_GeometryViewer();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);

		virtual void resetCamPose(void);
		virtual void setCamPose(const GVIEWER_CAM& camPose);
		virtual GVIEWER_CAM getCamPose(void);

	protected:
		// update thread
		virtual bool addAllGeometries(void);
		virtual void updateAllGeometries(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_GeometryViewer *)This)->update();
			return NULL;
		}

		// UI
		void updateCamProj(void);
		void updateCamPose(void);
		void camBound(const AxisAlignedBoundingBox &aabb);
		AxisAlignedBoundingBox createDefaultAABB(void);

		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_GeometryViewer *)This)->updateUI();
			return NULL;
		}

	protected:
		vector<GVIEWER_OBJ> m_vGO;

		O3DUI *m_pWin;
		UIState *m_pUIstate;
		_Thread *m_pTui;
		string m_dirSave;

		string m_pathRes;
		Visualizer m_vis;
		vInt2 m_vWinSize;
		string m_device;

		bool m_bFullScreen;
		bool m_bSceneCache;
		int m_wPanel;
		vFloat2 m_vBtnPadding;
		int m_mouseMode;
		vFloat2 m_vDmove;

		GVIEWER_CAM_PROJ m_camProj;
		GVIEWER_CAM m_cam;
		GVIEWER_CAM m_camDefault;
		GVIEWER_CAM m_camAuto;
		vFloat3 m_vCoR;
		AxisAlignedBoundingBox m_aabb;
	};

}
#endif
