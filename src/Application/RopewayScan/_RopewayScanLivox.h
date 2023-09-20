/*
 * _RopewayScanLivox.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_RopewayScan__RopewayScanLivox_H_
#define OpenKAI_src_Application_RopewayScan__RopewayScanLivox_H_

#include "../../3D/PointCloud/_PCstream.h"
#include "../../3D/_GeometryViewer.h"
#include "../../Navigation/_NavBase.h"
#include "../../Utility/BitFlag.h"
#include "../../Actuator/_ActuatorBase.h"
#include "../../LIDAR/Livox/_Livox.h"

namespace kai
{
	class _RopewayScanLivox : public _GeometryViewer
	{
	public:
		_RopewayScanLivox();
		virtual ~_RopewayScanLivox();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);

	protected:
		//point cloud
		virtual void scanReset(void);
		virtual void scanStart(void);
		virtual void scanUpdate(void);
		virtual void scanTake(void);
		virtual void scanStop(void);

		virtual void savePC(void);

		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_RopewayScanLivox *)This)->update();
			return NULL;
		}

		//attitude
		virtual void updateSlam(void);
		virtual void updateKinematics(void);
		static void *getUpdateKinematics(void *This)
		{
			((_RopewayScanLivox *)This)->updateKinematics();
			return NULL;
		}

		// handlers
		static void OnScanReset(void *pPCV, void* pD);
		static void OnScanSet(void *pPCV, void* pD);
		static void OnScanStart(void *pPCV, void* pD);
		static void OnScanStop(void *pPCV, void* pD);

		static void OnSavePC(void *pPCV, void* pD);

	protected:
		_NavBase *m_pNav;
		_Thread *m_pTk;

		vector<PointCloud> m_vPC;	// original point cloud data frames
		int m_nP;
		int m_nPmax;
		PointCloud* m_pPCprv;
		int m_iPprv;
		float m_rVoxel;
		string m_baseDir;
		string m_dir;

		LivoxCtrl m_livoxCtrl;
		BIT_FLAG m_fProcess;

		bool m_bScanning;
		int m_nTake;
	};

}
#endif
