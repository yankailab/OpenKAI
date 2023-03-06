/*
 * _VzScanCalib.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_VzScanCalib__VzScanCalib_H_
#define OpenKAI_src_Application_VzScanCalib__VzScanCalib_H_

#include "../../3D/PointCloud/_PCstream.h"
#include "../../3D/_GeometryViewer.h"
#include "../../Navigation/_NavBase.h"
#include "../../Utility/BitFlag.h"
#include "VzScanCalibUI.h"

namespace kai
{
	class _VzScanCalib : public _GeometryViewer
	{
	public:
		_VzScanCalib();
		virtual ~_VzScanCalib();

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
			((_VzScanCalib *)This)->update();
			return NULL;
		}

		//attitude
		virtual void updateSlam(void);
		virtual void updateKinematics(void);
		static void *getUpdateKinematics(void *This)
		{
			((_VzScanCalib *)This)->updateKinematics();
			return NULL;
		}

		// UI
		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_VzScanCalib *)This)->updateUI();
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

		int m_nPC;
		PointCloud m_PCprv;
		int m_iPprv;
		int m_nPprv;
		float m_rVoxel;
		string m_fNameCalib;

		VzCamCtrl m_camCtrl;
		BIT_FLAG m_fProcess;
	};

}
#endif
