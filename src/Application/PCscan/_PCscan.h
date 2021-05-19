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
		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_PCscan *)This)->updateUI();
			return NULL;
		}

		// handlers
		static void OnScan(void *pPCV, void* pD);
		static void OnOpenPC(void *pPCV, void* pD);
		static void OnCamSet(void *pPCV, void* pD);
		static void OnVoxelDown(void *pPCV, void* pD);
		static void OnHiddenRemove(void *pPCV, void* pD);
		static void OnResetPC(void *pPCV, void* pD);

		AxisAlignedBoundingBox createDefaultAABB(void);

	protected:
		_PCstream* m_pPS;
		_SlamBase *m_pSB;
		_Thread *m_pTk;

		float m_dHiddenRemove;
		AxisAlignedBoundingBox m_aabb;

		//filter flags
		BIT_FLAG m_fProcess;
	};

}
#endif
#endif
