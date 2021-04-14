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

		virtual bool startScan(void);
		virtual bool stopScan(void);

	protected:
		virtual void addUIpc(const PointCloud& pc);
		virtual void updateUIpc(const PointCloud& pc);
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
		static void OnBtnSavePC(void *pPCV, void* pD);
		static void OnBtnCamSet(void *pPCV, void* pD);

	protected:
		_PCstream* m_pPS;
		shared_ptr<visualizer::PCscanUI> m_spWin;
		string m_modelName;
		_Thread *m_pTk;
		_SlamBase *m_pSB;

		bool m_bSceneCache;
		float m_selectPointSize;

		pthread_mutex_t m_mutexScan;
		bool m_bScanning;
		AxisAlignedBoundingBox m_aabb;
	};

}
#endif
#endif
