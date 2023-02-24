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
#include "../../LIDAR/_VzensePC.h"
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
		virtual void scanReset(void);
		virtual void scanTake(void);
		virtual void savePC(void);

		virtual void updateCamCtrl(void);
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
		static void OnScanReset(void *pPCV, void* pD);
		static void OnScanTake(void *pPCV, void* pD);
		static void OnSavePC(void *pPCV, void* pD);
		static void OnOpenPC(void *pPCV, void* pD);
		static void OnCamSet(void *pPCV, void* pD);
		static void OnCamCtrl(void *pPCV, void* pD);

	protected:
		_NavBase *m_pNav;
		_Thread *m_pTk;

		PointCloud* m_pPCprv;
		PointCloud* m_pPCorig;
		int m_nPwPrv;
		int m_nPwOrig;
		float m_rVoxel;
		string m_fNameSavePC;

		VzCamCtrl m_camCtrl;
		VzCamCtrl m_camCtrlNew;
		
		//filter flags
		BIT_FLAG m_fProcess;
	};

}
#endif
