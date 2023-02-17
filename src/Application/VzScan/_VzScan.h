/*
 * _VzScan.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_VzScan__VzScan_H_
#define OpenKAI_src_Application_VzScan__VzScan_H_

#include "../../3D/PointCloud/_PCstream.h"
#include "../../3D/_GeometryViewer.h"
#include "../../Navigation/_NavBase.h"
#include "../../Utility/BitFlag.h"
#include "VzScanUI.h"

namespace kai
{
	class _VzScan : public _GeometryViewer
	{
	public:
		_VzScan();
		virtual ~_VzScan();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);

	protected:
		//point cloud
		virtual void updateProcess(void);
		virtual void scanReset(void);
		virtual void scanTake(void);

//		virtual void startScan(void);
//		virtual void stopScan(void);
		virtual void updateCamAuto(void);
		virtual void updateScan(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_VzScan *)This)->update();
			return NULL;
		}

		//attitude
		virtual void updateSlam(void);
		virtual void updateKinematics(void);
		static void *getUpdateKinematics(void *This)
		{
			((_VzScan *)This)->updateKinematics();
			return NULL;
		}

		// UI
		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_VzScan *)This)->updateUI();
			return NULL;
		}

		// handlers
//		static void OnScan(void *pPCV, void* pD);
		static void OnScanReset(void *pPCV, void* pD);
		static void OnScanTake(void *pPCV, void* pD);
		static void OnOpenPC(void *pPCV, void* pD);
		static void OnCamSet(void *pPCV, void* pD);
		static void OnVoxelDown(void *pPCV, void* pD);
		static void OnHiddenRemove(void *pPCV, void* pD);
		static void OnResetPC(void *pPCV, void* pD);


	protected:
		_NavBase *m_pNav;
		_Thread *m_pTk;

		int m_nPw;
		float m_dHiddenRemove;

		//filter flags
		BIT_FLAG m_fProcess;
	};

}
#endif
