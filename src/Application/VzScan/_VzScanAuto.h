/*
 * _VzScanAuto.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_VzScanAuto__VzScanAuto_H_
#define OpenKAI_src_Application_VzScanAuto__VzScanAuto_H_

#include "../../3D/PointCloud/_PCstream.h"
#include "../../3D/_GeometryViewer.h"
#include "../../Navigation/_NavBase.h"
#include "../../Utility/BitFlag.h"
#include "../../Actuator/_ActuatorBase.h"
#include "VzScanAutoUI.h"

namespace kai
{
	struct VZSCAN_ACTUATOR
	{
		float m_vTarget = 0.5;
		float m_v = 0.5;
		float m_dV = 0.01;
		int m_iAxis = 0;

		void setTarget(float v)
		{
			m_vTarget = v;
		}

		float update(void)
		{
			float d = m_vTarget - m_v;

			if(d <= m_dV)
				m_v = m_vTarget;
			else if(d > 0)
				m_v += m_dV;
			else
				m_v -= m_dV;

			return m_v;
		}

		bool bComplete(void)
		{
			return (m_v == m_vTarget);
		}
	};

	class _VzScanAuto : public _GeometryViewer
	{
	public:
		_VzScanAuto();
		virtual ~_VzScanAuto();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);

	protected:
		//point cloud
		virtual void scanReset(void);
		virtual void scanSet(void);
		virtual void scanStart(void);
		virtual void scanStop(void);

		virtual void savePC(void);

		virtual void updateCamCtrl(void);
		virtual void updateCamAuto(void);
		virtual void updateScan(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_VzScanAuto *)This)->update();
			return NULL;
		}

		//attitude
		virtual void updateSlam(void);
		virtual void updateKinematics(void);
		static void *getUpdateKinematics(void *This)
		{
			((_VzScanAuto *)This)->updateKinematics();
			return NULL;
		}

		// UI
		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_VzScanAuto *)This)->updateUI();
			return NULL;
		}

		// handlers
		static void OnScanReset(void *pPCV, void* pD);
		static void OnScanSet(void *pPCV, void* pD);
		static void OnScanStart(void *pPCV, void* pD);
		static void OnScanStop(void *pPCV, void* pD);

		static void OnSavePC(void *pPCV, void* pD);
		static void OnCamSet(void *pPCV, void* pD);
		static void OnCamCtrl(void *pPCV, void* pD);

	protected:
		_NavBase *m_pNav;
		_Thread *m_pTk;

		vector<PointCloud> m_vPC;	// original point cloud data frames
		int m_nP;
		int m_nPmax;
		PointCloud* m_pPCprv;
		int m_iPprv;
		float m_rVoxel;
		string m_fNameSavePC;

		VzCamCtrl m_camCtrl;
		BIT_FLAG m_fProcess;

		int m_iTake;
		_ActuatorBase* m_pAct;
		VZSCAN_ACTUATOR m_actH;
		VZSCAN_ACTUATOR m_actV;
		VzScanSet m_scanSet;
	};

}
#endif
