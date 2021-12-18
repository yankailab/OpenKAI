/*
 * _GeometryViewer.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D__GeometryViewer_H_
#define OpenKAI_src_3D__GeometryViewer_H_

#include "PointCloud/_PCframe.h"
#include "../UI/O3DUI.h"

namespace kai
{
    enum PC_THREAD_ACTION
	{
		pc_Scanning = 0,
		pc_ScanStart = 1,
		pc_ScanStop = 2,
		pc_VoxelDown = 3,
		pc_HiddenRemove = 4,
		pc_ResetPC = 5,
		pc_CamAuto = 6,
		pc_RefreshCol = 7,
	};

	struct CAM_PROJ
	{
		float m_fov = 70.0;
		vFloat2 m_vNF = {0, FLT_MAX};	//near far plane
		uint8_t m_fovType = 0;
	};

	struct PC_CAM
	{
		vFloat3 m_vLookAt = {0,0,0};
		vFloat3 m_vEye = {0,0,1};
		vFloat3 m_vUp = {0,1,0};
	};

	class _GeometryViewer : public _PCframe
	{
	public:
		_GeometryViewer();
		virtual ~_GeometryViewer();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);

		virtual void resetCamPose(void);

	protected:
		//data
		void addUIpc(const PointCloud& pc);
		void updateUIpc(const PointCloud& pc);
		void removeUIpc(void);
		void readAllPC(void);
		void addDummyDome(PointCloud* pPC, int n, float r, Vector3d vCol = {0,0,0});
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_GeometryViewer *)This)->update();
			return NULL;
		}

		//UI
		void updateCamProj(void);
		void updateCamPose(void);
		void camBound(const AxisAlignedBoundingBox& aabb);
		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_GeometryViewer *)This)->updateUI();
			return NULL;
		}

	protected:
		O3DUI* m_pWin;
		UIState* m_pUIstate;
		_Thread *m_pTui;
		string m_modelName;

		string m_pathRes;
		Visualizer m_vis;
		vInt2 m_vWinSize;
		string m_device;

		bool m_bFullScreen;
		bool m_bSceneCache;
		int	m_wPanel;
		vFloat2 m_vBtnPadding;
		int m_mouseMode;
		vFloat2 m_vDmove;
		float m_rDummyDome;

		CAM_PROJ m_camProj;
		PC_CAM m_cam;
		PC_CAM m_camDefault;
		PC_CAM m_camAuto;
		vFloat3 m_vCoR;

		vector<_GeometryBase *> m_vpGB;
	};

}
#endif
