/*
 * _PCscan.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_PCscan__PCscan_H_
#define OpenKAI_src_Application_PCscan__PCscan_H_
#ifdef USE_OPEN3D
#include "../../PointCloud/_PCstream.h"
#include "../../PointCloud/_PCviewer.h"
#include "../../SLAM/_SlamBase.h"
#include "../../Utility/BitFlag.h"
#include "PCscanUI.h"

namespace kai
{
	class _PCscan : public _PCviewer
	{
	public:
		_PCscan();
		virtual ~_PCscan();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);

	protected:
		//point cloud
		void addUIpc(const PointCloud& pc);
		void updateUIpc(const PointCloud& pc);
		void removeUIpc(void);
		
		virtual void updateProcess(void);
		virtual void startScan(void);
		virtual void stopScan(void);
		virtual void updateCamAuto(void);
		virtual void updateScan(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_PCscan *)This)->update();
			return NULL;
		}

		//attitude
		virtual void updateSlam(void);
		virtual void updateKinematics(void);
		static void *getUpdateKinematics(void *This)
		{
			((_PCscan *)This)->updateKinematics();
			return NULL;
		}

		// UI
		void updateCamProj(void);
		void updateCamPose(void);
		void camBound(const AxisAlignedBoundingBox& aabb);
		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_PCscan *)This)->updateUI();
			return NULL;
		}

		// handlers
		static void OnBtnScan(void *pPCV, void* pD);
		static void OnBtnOpenPC(void *pPCV, void* pD);
		static void OnBtnCamSet(void *pPCV, void* pD);
		static void OnVoxelDown(void *pPCV, void* pD);
		static void OnBtnHiddenRemove(void *pPCV, void* pD);
		static void OnBtnResetPC(void *pPCV, void* pD);

		AxisAlignedBoundingBox createDefaultAABB(void);

	protected:
		_PCstream* m_pPS;
		O3DUI* m_pWin;
		UIState* m_pUIstate;
		string m_modelName;

		_SlamBase *m_pSB;
		_Thread *m_pTk;

		bool m_bFullScreen;
		bool m_bSceneCache;
		int	m_wPanel;
		int m_mouseMode;
		vFloat2 m_vDmove;
		float m_rDummyDome;
		float m_dHiddenRemove;
		AxisAlignedBoundingBox m_aabb;

		//filter flags
		BIT_FLAG m_fProcess;
	};

}
#endif
#endif
