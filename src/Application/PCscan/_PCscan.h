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
	namespace
	{
		static const int pcfScanning = 0;
		static const int pcfScanStart = 1;
		static const int pcfScanStop = 2;
		static const int pcfVoxelDown = 3;
		static const int pcfHiddenRemove = 4;
		static const int pcfResetPC = 5;
		static const int pcfCamAuto = 6;
	}

	class _PCscan : public _PCviewer
	{
	public:
		_PCscan();
		virtual ~_PCscan();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);

	protected:
		virtual void addUIpc(const PointCloud& pc);
		virtual void updateUIpc(const PointCloud& pc);
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

		virtual void updateSlam(void);
		virtual void updateKinematics(void);
		static void *getUpdateKinematics(void *This)
		{
			((_PCscan *)This)->updateKinematics();
			return NULL;
		}

		virtual void updateCamProj(void);
		virtual void updateCamPose(void);
		virtual void camBound(const AxisAlignedBoundingBox& aabb);
		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_PCscan *)This)->updateUI();
			return NULL;
		}

		static void OnBtnScan(void *pPCV, void* pD);
		static void OnBtnOpenPC(void *pPCV, void* pD);
		static void OnBtnCamSet(void *pPCV, void* pD);
		static void OnVoxelDown(void *pPCV, void* pD);
		static void OnBtnHiddenRemove(void *pPCV, void* pD);
		static void OnBtnResetPC(void *pPCV, void* pD);

	protected:
		_PCstream* m_pPS;
		shared_ptr<visualizer::PCscanUI> m_spWin;
		visualizer::UIState* m_pUIstate;
		string m_modelName;
		_Thread *m_pTk;
		_SlamBase *m_pSB;

		bool m_bSceneCache;
		float m_rDummyDome;
		float m_dHiddenRemove;
		AxisAlignedBoundingBox m_aabb;

		//filter flags
		BIT_FLAG m_fProcess;
	};

}
#endif
#endif
